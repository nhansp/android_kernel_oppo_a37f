#
# Makefile for the OPPO Synaptics touchscreen driver.
#
# A37f uses the S3203 panel (compatible "synaptics-s3203"); only the 3203
# variant is built here. The other OPPO variants (3310/3508/4291) each define
# the same tpd_driver_init/probe symbols and would collide at link.
#
obj-$(CONFIG_SYNAPTICS_OPPO) += synaptics_oppo_driver_3203.o
obj-$(CONFIG_SYNAPTICS_OPPO) += synaptics_redremote.o
