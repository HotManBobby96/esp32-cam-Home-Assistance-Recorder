#importing nessacary libraries
import asyncio
from bleak import BleakClient

ADDRESS = "" # replace with your current MAC Adress
CHAR_UUID = ""  # UUID from your sketch

async def main():
    async with BleakClient(ADDRESS) as client:
        await client.write_gatt_char(CHAR_UUID, b"record")
        print("Trigger sent")

asyncio.run(main())
 