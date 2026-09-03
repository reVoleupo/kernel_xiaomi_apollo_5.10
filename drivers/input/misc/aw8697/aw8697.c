// SPDX-License-Identifier: GPL-2.0
/*
 * AW8697 haptic vibrator driver for Xiaomi apollo
 * Minimal 5.10 mainline port
 */
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/regmap.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#define AW8697_REG_RESET		0x00
#define AW8697_REG_DEVICE_ID		0x01
#define AW8697_REG_MODE_CTRL		0x02
#define AW8697_REG_PLAY_ENABLE		0x03

struct aw8697_chip {
	struct i2c_client	*client;
	struct regmap		*regmap;
	struct input_dev	*input;
	struct workqueue_struct	*workqueue;
	struct work_struct	play_work;
	bool			playing;
};

static void aw8697_play_work(struct work_struct *work)
{
	struct aw8697_chip *chip = container_of(work, struct aw8697_chip, play_work);

	regmap_write(chip->regmap, AW8697_REG_PLAY_ENABLE, 0x01);
	usleep_range(50000, 60000);
	regmap_write(chip->regmap, AW8697_REG_PLAY_ENABLE, 0x00);
	chip->playing = false;
}

static int aw8697_play_effect(struct input_dev *input, void *data,
			      struct ff_effect *effect)
{
	struct aw8697_chip *chip = input_get_drvdata(input);

	if (effect->type != FF_RUMBLE)
		return 0;

	if (!chip->playing) {
		chip->playing = true;
		queue_work(chip->workqueue, &chip->play_work);
	}
	return 0;
}

static bool aw8697_volatile_reg(struct device *dev, unsigned int reg)
{
	return true;
}

static const struct regmap_config aw8697_regmap = {
	.reg_bits	= 8,
	.val_bits	= 8,
	.max_register	= 0xff,
	.volatile_reg	= aw8697_volatile_reg,
};

static int aw8697_probe(struct i2c_client *client)
{
	struct aw8697_chip *chip;
	int ret;

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;
	i2c_set_clientdata(client, chip);

	chip->regmap = devm_regmap_init_i2c(client, &aw8697_regmap);
	if (IS_ERR(chip->regmap))
		return PTR_ERR(chip->regmap);

	chip->input = devm_input_allocate_device(&client->dev);
	if (!chip->input)
		return -ENOMEM;

	chip->input->name = "aw8697-haptic";
	chip->input->id.bustype = BUS_I2C;
	input_set_drvdata(chip->input, chip);
	input_set_capability(chip->input, EV_FF, FF_RUMBLE);

	ret = input_ff_create_memless(chip->input, NULL, aw8697_play_effect);
	if (ret)
		return ret;

	chip->workqueue = create_singlethread_workqueue("aw8697");
	if (!chip->workqueue)
		return -ENOMEM;
	INIT_WORK(&chip->play_work, aw8697_play_work);

	ret = input_register_device(chip->input);
	if (ret) {
		destroy_workqueue(chip->workqueue);
	return 0;
		return ret;
	}

	/* Reset the chip */
	regmap_write(chip->regmap, AW8697_REG_RESET, 0x01);

	dev_info(&client->dev, "AW8697 haptic probed\n");
	return 0;
}

static int aw8697_remove(struct i2c_client *client)
{
	struct aw8697_chip *chip = i2c_get_clientdata(client);
	destroy_workqueue(chip->workqueue);
	return 0;
}

static const struct of_device_id aw8697_match[] = {
	{ .compatible = "awinic,aw8697" },
	{}
};
MODULE_DEVICE_TABLE(of, aw8697_match);

static struct i2c_driver aw8697_driver = {
	.driver = {
		.name = "aw8697-haptic",
		.of_match_table = aw8697_match,
	},
	.probe_new = aw8697_probe,
	.remove = aw8697_remove,
};
module_i2c_driver(aw8697_driver);

MODULE_DESCRIPTION("AW8697 haptic vibrator driver");
MODULE_LICENSE("GPL");
