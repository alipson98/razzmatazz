import asyncio
import platform
import struct
import csv

from bleak import BleakClient, BleakScanner

LED_UUID = "19b10001-e8f2-537e-4f6c-d104768a1214"
COUNT_UUID = "19b10001-e8f2-537e-4f6c-d104768a1215"
IMU_UUID = "19b10001-e8f2-537e-4f6c-d104768a1216"

def callback(sender: int, data: bytearray):
    print(data)
    unpacked = struct.unpack("<LLLL", data)
    print(f"{sender}: {unpacked}")
    with open("testdata.csv", 'a') as outfile:
        csv_out = csv.writer(outfile)
        csv_out.writerow(unpacked)
    return

async def print_services(mac_addr: str):
    device = await BleakScanner.find_device_by_address(mac_addr)
    async with BleakClient(device) as client:
        await client.start_notify(IMU_UUID, callback)
        while(client.is_connected):
            # print("hi")
            continue
        # print(type(client))


        svcs = await client.get_services()
        print("Services:", svcs.services['19b10000-e8f2-537e-4f6c-d104768a1214'].characteristics)

        for characteristic in svcs.services['19b10000-e8f2-537e-4f6c-d104768a1214'].characteristics:
            print(characteristic.descriptors)
            # desc1 = characteristic.get_descriptor(13)
            # desc2 = characteristic.get_descriptor(16)
            # print(desc1)
            # print(desc2)
        
        led = await client.read_gatt_char(LED_UUID)
        count = await client.read_gatt_char(COUNT_UUID)
        imu = await client.read_gatt_char(IMU_UUID)
        print("led: "+str(led))
        print("count: "+str(count))
        print("imu: "+str(imu))
        await client.write_gatt_char(LED_UUID, bytearray(b'\x01'))
        count = await client.read_gatt_char(COUNT_UUID)
        unpacked = struct.unpack("<LLLL", imu) # indexed at 0 bc unpack returns a tuple
        print(type(unpacked))
        print("led: "+str(led))
        print("count: "+str(count))
        print("unpacked: "+str(unpacked))


mac_addr = ( # TODO: set MAC addr and UUID based on bleak_get_services.py and MAC addr in arduino sketch
    "24:0a:c4:ac:e0:82"
    if platform.system() != "Darwin"
    else "CDCE1F62-3600-4EA7-B0D5-7A3D1AA0A05E"
)
loop = asyncio.get_event_loop()
loop.run_until_complete(print_services(mac_addr))