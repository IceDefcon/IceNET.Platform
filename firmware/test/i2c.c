#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define I2C_DEVICE_ADDRESS 0x69 // The address of the device 
#define I2C_BUS_NUMBER 2        // The I2C bus for P9_19 and P9_20

static struct i2c_client *i2c_client;

static int __init i2c_module_init(void)
{
    struct i2c_adapter *adapter;
    struct i2c_board_info board_info;

    // Get the I2C adapter corresponding to the bus number
    adapter = i2c_get_adapter(I2C_BUS_NUMBER);

    // Populate the board_info structure with device-specific information
    memset(&board_info, 0, sizeof(struct i2c_board_info));
    strlcpy(board_info.type, "your-device-name", I2C_NAME_SIZE);   // Replace with your device name
    board_info.addr = I2C_DEVICE_ADDRESS;

    // Create an I2C client for the device
    i2c_client = i2c_new_device(adapter, &board_info);
    // Release the I2C adapter
    i2c_put_adapter(adapter);

    if (!i2c_client) {
        printk(KERN_ERR "Failed to create I2C device\n");
        return -ENODEV;
    }
    else printk(KERN_INFO "I2C device created\n");

    char read_buffer[1];
    int i;
    int ret;

    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x00, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x01, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x02, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x03, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }


    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x04, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x05, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x06, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x07, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x08, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x09, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x0a, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x0b, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x0c, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x0d, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x0e, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x0f, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }




    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x10, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x11, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x12, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x13, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }


    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x14, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x15, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x16, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x17, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x18, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x19, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x1a, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x1b, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x1c, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x1d, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x1e, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }



    ret = i2c_smbus_read_i2c_block_data(i2c_client, 0x1f, sizeof(read_buffer), read_buffer);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from I2C device\n");
        i2c_unregister_device(i2c_client);
        return ret;
    }

    for (i = 0; i < sizeof(read_buffer); i++) {
        printk(KERN_INFO "Read data[%d]: 0x%02x\n", i, read_buffer[i]);
    }


    // int ret = i2c_smbus_write_byte_data(i2c_client, 0x70, 0x01);
    // if (ret < 0) {
    //     printk(KERN_ERR "Failed to write data to I2C device\n");
    //     i2c_unregister_device(i2c_client);
    //     return ret;
    // }

    return 0;
}

static void __exit i2c_module_exit(void)
{
    // Delete the I2C client
    i2c_unregister_device(i2c_client);
}

module_init(i2c_module_init);
module_exit(i2c_module_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C Kernel Module Example");
