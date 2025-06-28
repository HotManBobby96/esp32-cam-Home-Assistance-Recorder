import asyncio
from bleak import BleakClient, BleakScanner  # type: ignore
import time

SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
address = "FC:E8:C0:A0:8A:AE"
connectTime = 10

async def main(): 

    while True: 
        userInput = input("Run again? Enter Y or N") # getting the usre input
        if userInput.lower() == 'y':
            devices = await BleakScanner.discover()
            for d in devices:
                print(d)
        else: 
            return 0

        async with BleakClient(address) as client:
            connected = await client.is_connected()
            print(f"Connected: {connected}")
            print(f"MacAddress: {address}")

            await asyncio.sleep(1)

            value = await client.read_gatt_char(CHARACTERISTIC_UUID)
            print(f"Read: {value.decode()}")

            await client.write_gatt_char(CHARACTERISTIC_UUID, b"Hello from PC!")
            print("Wrote: Hello from PC!")

            await asyncio.sleep(connectTime)

            await client.disconnect()
            print(f"Disconnected from: {address}")


asyncio.run(main())
