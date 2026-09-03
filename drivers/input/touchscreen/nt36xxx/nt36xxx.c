// SPDX-License-Identifier: GPL-2.0
/*
 * Novatek NT36xxx SPI touchscreen driver for Xiaomi apollo
 * Minimal 5.10 mainline port
 */
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>

#define NVT_MAX_TOUCH_POINTS	10
#define NVT_TOUCH_DATA_LEN	(NVT_MAX_TOUCH_POINTS * 6 + 4)

struct nvt_ts_data {
	struct spi_device	*spi;
	struct input_dev	*input;
	struct gpio_desc	*reset_gpio;
	struct gpio_desc	*irq_gpio;
	struct regulator	*vdd;
	struct regulator	*vcc;
	u8			touch_data[NVT_TOUCH_DATA_LEN];
	bool			suspended;
};

static int nvt_spi_read(struct nvt_ts_data *ts, u8 *buf, u16 len)
{
	struct spi_transfer xfer = {
		.rx_buf = buf,
		.len = len,
	};
	return spi_sync_transfer(ts->spi, &xfer, 1);
}

static int __maybe_unused nvt_spi_write(struct nvt_ts_data *ts, const u8 *buf, u16 len)
{
	struct spi_transfer xfer = {
		.tx_buf = buf,
		.len = len,
	};
	return spi_sync_transfer(ts->spi, &xfer, 1);
}

static irqreturn_t nvt_ts_irq(int irq, void *dev_id)
{
	struct nvt_ts_data *ts = dev_id;
	int i, ret;
	u8 *data = ts->touch_data;

	ret = nvt_spi_read(ts, data, NVT_TOUCH_DATA_LEN);
	if (ret < 0)
		return IRQ_HANDLED;

	/* Parse touch points */
	for (i = 0; i < NVT_MAX_TOUCH_POINTS; i++) {
		u8 status = data[i * 6 + 2];
		u16 x = ((data[i * 6 + 3] & 0x0f) << 8) | data[i * 6 + 4];
		u16 y = ((data[i * 6 + 5] & 0x0f) << 8) | data[i * 6 + 6];
		u8 id = (data[i * 6 + 3] >> 4) & 0x0f;

		if (status & 0x01) {
			input_mt_slot(ts->input, id);
			input_mt_report_slot_state(ts->input, MT_TOOL_FINGER, true);
			input_report_abs(ts->input, ABS_MT_POSITION_X, x);
			input_report_abs(ts->input, ABS_MT_POSITION_Y, y);
		}
	}

	input_mt_sync_frame(ts->input);
	input_sync(ts->input);

	return IRQ_HANDLED;
}

static void nvt_ts_power_on(struct nvt_ts_data *ts)
{
	if (ts->vdd)
		(void)regulator_enable(ts->vdd);
	if (ts->vcc)
		(void)regulator_enable(ts->vcc);
	usleep_range(10000, 15000);

	if (ts->reset_gpio) {
		gpiod_set_value_cansleep(ts->reset_gpio, 1);
		usleep_range(5000, 10000);
		gpiod_set_value_cansleep(ts->reset_gpio, 0);
		msleep(100);
	}
}

static void nvt_ts_power_off(struct nvt_ts_data *ts)
{
	if (ts->reset_gpio)
		gpiod_set_value_cansleep(ts->reset_gpio, 1);
	if (ts->vcc)
		(void)regulator_disable(ts->vcc);
	if (ts->vdd)
		(void)regulator_disable(ts->vdd);
}

static int nvt_ts_probe(struct spi_device *spi)
{
	struct nvt_ts_data *ts;
	int ret;

	ts = devm_kzalloc(&spi->dev, sizeof(*ts), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;

	ts->spi = spi;
	spi_set_drvdata(spi, ts);

	spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;
	ret = spi_setup(spi);
	if (ret)
		return ret;

	/* Get GPIOs */
	ts->reset_gpio = devm_gpiod_get_optional(&spi->dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(ts->reset_gpio))
		return PTR_ERR(ts->reset_gpio);

	/* Get regulators */
	ts->vdd = devm_regulator_get_optional(&spi->dev, "vdd");
	if (IS_ERR(ts->vdd))
		ts->vdd = NULL;
	ts->vcc = devm_regulator_get_optional(&spi->dev, "vcc");
	if (IS_ERR(ts->vcc))
		ts->vcc = NULL;

	/* Power on */
	nvt_ts_power_on(ts);

	/* Allocate input device */
	ts->input = devm_input_allocate_device(&spi->dev);
	if (!ts->input) {
		ret = -ENOMEM;
		goto err_power_off;
	}

	ts->input->name = "Novatek NT36xxx Touchscreen";
	ts->input->id.bustype = BUS_SPI;
	ts->input->dev.parent = &spi->dev;

	input_set_capability(ts->input, EV_ABS, ABS_MT_POSITION_X);
	input_set_capability(ts->input, EV_ABS, ABS_MT_POSITION_Y);
	input_set_abs_params(ts->input, ABS_MT_POSITION_X, 0, 1080, 0, 0);
	input_set_abs_params(ts->input, ABS_MT_POSITION_Y, 0, 2400, 0, 0);

	ret = input_mt_init_slots(ts->input, NVT_MAX_TOUCH_POINTS,
				  INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);
	if (ret)
		goto err_power_off;

	ret = input_register_device(ts->input);
	if (ret)
		goto err_power_off;

	/* Request IRQ */
	ret = devm_request_threaded_irq(&spi->dev, spi->irq, NULL,
					nvt_ts_irq, IRQF_ONESHOT,
					"nvt_ts", ts);
	if (ret)
		goto err_power_off;

	dev_info(&spi->dev, "NT36xxx touchscreen probed\n");
	return 0;

err_power_off:
	nvt_ts_power_off(ts);
	return ret;
}

static int nvt_ts_remove(struct spi_device *spi)
{
	struct nvt_ts_data *ts = spi_get_drvdata(spi);
	nvt_ts_power_off(ts);
	return 0;
}

static int __maybe_unused nvt_ts_suspend(struct device *dev)
{
	struct nvt_ts_data *ts = dev_get_drvdata(dev);
	if (!ts->suspended) {
		disable_irq(ts->spi->irq);
		nvt_ts_power_off(ts);
		ts->suspended = true;
	}
	return 0;
}

static int __maybe_unused nvt_ts_resume(struct device *dev)
{
	struct nvt_ts_data *ts = dev_get_drvdata(dev);
	if (ts->suspended) {
		nvt_ts_power_on(ts);
		enable_irq(ts->spi->irq);
		ts->suspended = false;
	}
	return 0;
}

static SIMPLE_DEV_PM_OPS(nvt_ts_pm_ops, nvt_ts_suspend, nvt_ts_resume);

static const struct of_device_id nvt_ts_match[] = {
	{ .compatible = "novatek,nt36xxx" },
	{ .compatible = "novatek,nt36523" },
	{ .compatible = "novatek,nt36675" },
	{}
};
MODULE_DEVICE_TABLE(of, nvt_ts_match);

static struct spi_driver nvt_ts_driver = {
	.driver = {
		.name = "nt36xxx-ts",
		.of_match_table = nvt_ts_match,
		.pm = &nvt_ts_pm_ops,
	},
	.probe = nvt_ts_probe,
	.remove = nvt_ts_remove,
};
module_spi_driver(nvt_ts_driver);

MODULE_DESCRIPTION("Novatek NT36xxx SPI touchscreen driver");
MODULE_LICENSE("GPL");
