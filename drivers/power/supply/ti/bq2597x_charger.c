// SPDX-License-Identifier: GPL-2.0
/*
 * TI BQ2597x charge pump driver for Xiaomi apollo
 * Minimal 5.10 mainline port
 */
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/power_supply.h>
#include <linux/regmap.h>
#include <linux/interrupt.h>

#define BQ2597X_REG_MANUFACTURER_ID	0x00
#define BQ2597X_REG_DEVICE_ID		0x01

struct bq2597x_chip {
	struct i2c_client	*client;
	struct regmap		*regmap;
	struct power_supply	*charger;
};

static enum power_supply_property bq2597x_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT,
	POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE,
};

static int bq2597x_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct bq2597x_chip *chip = power_supply_get_drvdata(psy);
	unsigned int reg;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		regmap_read(chip->regmap, 0x08, &reg);
		val->intval = (reg & 0x30) ? POWER_SUPPLY_STATUS_CHARGING :
					     POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		regmap_read(chip->regmap, 0x08, &reg);
		val->intval = (reg & 0x80) ? 1 : 0;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = 5000000;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = 250;
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT:
		val->intval = 3000000;
		break;
	case POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE:
		val->intval = 4400000;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int bq2597x_set_property(struct power_supply *psy,
	enum power_supply_property psp, const union power_supply_propval *val)
{
	return -EINVAL;
}

static int bq2597x_property_is_writeable(struct power_supply *psy,
	enum power_supply_property psp)
{
	return 0;
}

static const struct power_supply_desc bq2597x_desc = {
	.name			= "bq2597x-charger",
	.type			= POWER_SUPPLY_TYPE_USB,
	.properties		= bq2597x_props,
	.num_properties		= ARRAY_SIZE(bq2597x_props),
	.get_property		= bq2597x_get_property,
	.set_property		= bq2597x_set_property,
	.property_is_writeable	= bq2597x_property_is_writeable,
};

static bool bq2597x_volatile_reg(struct device *dev, unsigned int reg)
{
	return true;
}

static const struct regmap_config bq2597x_regmap = {
	.reg_bits	= 8,
	.val_bits	= 8,
	.max_register	= 0x40,
	.volatile_reg	= bq2597x_volatile_reg,
};

static int bq2597x_probe(struct i2c_client *client)
{
	struct bq2597x_chip *chip;
	struct power_supply_config cfg = {};
	int ret __maybe_unused;

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;
	i2c_set_clientdata(client, chip);

	chip->regmap = devm_regmap_init_i2c(client, &bq2597x_regmap);
	if (IS_ERR(chip->regmap))
		return PTR_ERR(chip->regmap);

	cfg.drv_data = chip;
	chip->charger = devm_power_supply_register(&client->dev,
						   &bq2597x_desc, &cfg);
	if (IS_ERR(chip->charger))
		return PTR_ERR(chip->charger);

	dev_info(&client->dev, "BQ2597x charger probed\n");
	return 0;
}

static const struct of_device_id bq2597x_match[] = {
	{ .compatible = "ti,bq25970" },
	{ .compatible = "ti,bq2597x" },
	{}
};
MODULE_DEVICE_TABLE(of, bq2597x_match);

static struct i2c_driver bq2597x_driver = {
	.driver = {
		.name = "bq2597x-charger",
		.of_match_table = bq2597x_match,
	},
	.probe_new = bq2597x_probe,
};
module_i2c_driver(bq2597x_driver);

MODULE_DESCRIPTION("TI BQ2597x charge pump driver");
MODULE_LICENSE("GPL");
