// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/moduleparam.h>
#include <linux/list.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/bug.h>

static unsigned int repeat_count = 1;
module_param(repeat_count, uint, 0644);

struct event_data {
	struct list_head list;
	ktime_t timestamp;
};

static LIST_HEAD(hello_list);

static void __exit hello_exit(void)
{
	struct event_data *event, *tmp;
	int i = 1;

	pr_info("Starting module cleanup and printing timestamps...\n");

	list_for_each_entry_safe(event, tmp, &hello_list, list) {
		pr_info("Event %d time (ns): %lld\n",
		       i++, ktime_to_ns(event->timestamp));
		list_del(&event->list);
		kfree(event);
	}

	pr_info("Module cleanup finished.\n");
}

static int __init hello_init(void)
{
	int i;
	struct event_data *event;

	if (repeat_count > 10) {
		pr_warn("BUG_ON triggered for repeat_count = %u\n",
			repeat_count);
		BUG_ON(1);
		return -EINVAL;
	}

	if (repeat_count == 0 || (repeat_count >= 5 && repeat_count <= 10))
		pr_warn("Warning: repeat_count is %u\n", repeat_count);

	INIT_LIST_HEAD(&hello_list);

	for (i = 1; i <= repeat_count; i++) {
		if (i == 5 && repeat_count >= 5) {
			pr_warn("Simulating kmalloc failure for 5th message\n");
			BUG_ON(1);
			event = NULL;
		} else {
			event = kmalloc(sizeof(struct event_data), GFP_KERNEL);
			if (!event)
				return -ENOMEM;
		}

		if (event) {
			event->timestamp = ktime_get();
			list_add_tail(&event->list, &hello_list);
		}
		pr_info("Hello, world! (%d/%d)\n", i, repeat_count);
	}

	pr_info("Module loaded with repeat_count = %u\n", repeat_count);
	return 0;
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Siukalo Diana Group IO-34");
MODULE_DESCRIPTION("Lab 6 module");
