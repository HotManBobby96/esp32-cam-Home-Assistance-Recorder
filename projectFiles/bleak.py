#importing nessacary libraries
import asyncio
from bleak import BleakClient, BleakScanner

SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

async def main(): 
    devices = await BleakScanner.discover()
    for d in devices:
        print(d)
    

    address = input("enter MAC adress") # get this later from esp32 to know where it is at

    # Thabk you stack overflow
    async with BleakClient(address) as client:
        connected = await client.is_connected()
        print(f"Connected: {connected}")

        value = await client.read_gatt_char(CHARACTERISTIC_UUID)
        print(f"Read: {value.decode()}")

        await client.write_gatt_char(CHARACTERISTIC_UUID, b"Hello from PC!")
        print("Wrote: Hello from PC!")

asyncio.run(main())