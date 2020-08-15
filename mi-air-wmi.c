// SPDX-License-Identifier: GPL-2.0
/* WMI driver for Xiaomi Mi Air Laptops */

#include <linux/acpi.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/wmi.h>
#include <linux/input/sparse-keymap.h>
#include <uapi/linux/input-event-codes.h>

#define EVENT_GUID1 "ABBC0F74-8EA1-11D1-00A0-C90629100000"

static const struct key_entry mi_air_wmi_keymap[] = {
	{ KE_KEY, 0x1, {  } }, /* fn lock on */
	{ KE_KEY, 0x2, {  } }, /* fn lock off */
	{ KE_KEY, 0x3, { KEY_TOUCHPAD_ON } }, /* fn + space */
	{ KE_KEY, 0x4, { KEY_TOUCHPAD_OFF } }, /* fn + space */
	{ KE_KEY, 0x5, {  } }, /* capslock on */
	{ KE_KEY, 0x6, {  } }, /* capslock off */
	{ KE_KEY, 0x7, { KEY_PROG1 } }, /* double fn */
	{ KE_KEY, 0x8, { KEY_PROG2 } }, /* fn + f7 */
	{ KE_END, 0 }
};

static int mi_air_wmi_probe(struct wmi_device *wdev, const void *context)
{
	struct input_dev *input_dev;
	int err;

	if (wdev == NULL)
		return -EINVAL;

	input_dev = devm_input_allocate_device(&wdev->dev);
	if (input_dev == NULL)
		return -ENOMEM;

	input_dev->name = "Xiaomi Mi Air WMI keys";
	input_dev->phys = "wmi/input0";
	dev_set_drvdata(&wdev->dev, input_dev);

	err = sparse_keymap_setup(input_dev,
			mi_air_wmi_keymap, NULL);
	if (err) {
		pr_err("Unable to setup input device keymap\n");
		return err;
	}

	return input_register_device(input_dev);
}

static void mi_air_wmi_notify(struct wmi_device *wdev, union acpi_object *obj)
{
	struct input_dev *input_dev;

	if (wdev == NULL || obj == NULL)
		return;

	if (obj->type != ACPI_TYPE_INTEGER) {
		pr_warn("Unknown event type: %d", obj->type);
		return;
	}

	input_dev = dev_get_drvdata(&wdev->dev);
	if (input_dev == NULL)
		return;

	sparse_keymap_report_event(input_dev,
				obj->integer.value, 1, true);
}

static const struct wmi_device_id mi_air_wmi_id_table[] = {
	{ .guid_string = EVENT_GUID1 },

	/* Terminating entry */
	{ }
};

static struct wmi_driver mi_air_wmi_driver = {
	.driver = {
		.name = "xiaomi-mi-air-wmi",
	},
	.id_table = mi_air_wmi_id_table,
	.probe = mi_air_wmi_probe,
	.notify = mi_air_wmi_notify,
};

module_wmi_driver(mi_air_wmi_driver);

MODULE_DEVICE_TABLE(wmi, mi_air_wmi_id_table);
MODULE_AUTHOR("Mikalai Ramanovich");
MODULE_DESCRIPTION("Xiaomi Mi Air WMI driver");
MODULE_LICENSE("GPL v2");
