#importing nessacary libraries
import asyncio
import time
import base64
from bleak import BleakClient, BleakScanner # type: ignore
from datetime import datetime

SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b" #Matching the same as defined on arduino
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
address = "FC:E8:C0:A0:8A:AE" # Hard coding the macaddress allows the camera to connect twice as fast cutting the time delay in half

picturePath = "C:\\Users\\PCHS_BPA\\Desktop\\esp32-cam-Home-Assistance-Recorder\\projectFiles\\pictures" # saving images from the thingy mobob

async def main(): 
    devices = await BleakScanner.discover()
    for d in devices:
        print(d) # Finding all the devices around the area


    # Thabk you stack overflow
    async with BleakClient(address) as client: #Finding the macaddress
        connected = await client.is_connected()
        print(f"Connected: {connected}")
        print(f"MacAddress: {address}")

        await asyncio.sleep(1) # ensuring the connection before we try and do anything (What the fuck am I doing)

        value = await client.read_gatt_char(CHARACTERISTIC_UUID)
        print(f"Read: {value.decode()}")

        await client.write_gatt_char(CHARACTERISTIC_UUID, b"Hello from PC!")
        print("Wrote: Hello from PC!")

        await asyncio.sleep(60) # waits for 15 seconds giving the arduino time to register 

        await client.disconnect() # disconecs to connect later making it ble server
        print(f"disconnect from: {address}")


#10 Seconds to connect to the device

asyncio.run(main())