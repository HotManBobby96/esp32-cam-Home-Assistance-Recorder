#importing nessacary libraries
import asyncio
import time
import base64
from bleak import BleakClient, BleakScanner # type: ignore
from datetime import datetime

SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b" #Matching the same as defined on arduino
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
address = "00:4B:12:96:F2:2A" # Hard coding the macaddress allows the camera to connect twice as fast cutting the time delay in half

picturePath = "C:\\Users\\PCHS_BPA\\Desktop\\esp32-cam-Home-Assistance-Recorder\\projectFiles\\pictures" # saving images from the thingy mobob

imageBase = ""

data = b'\x01\x02\x03\x04' # sending bytes to the arudino for it know were connected 

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

        #setting this up for getting the dealio 
        image_chunks = []
        prevChunk = None
        counter = 0

        #write to the arudino and let it know toue ready
        if client.is_connected:
            print("Connected to the server")
            await client.write_gatt_char(CHARACTERISTIC_UUID, data)
            print("Data Written (ready)")
        else:
            print("Failed to connect and write")

        #getting chunks from the arduino and decoding them
        print("Running while true statement")
        while True:
            chunk = await client.read_gatt_char(CHARACTERISTIC_UUID)

            #Write to the arduino and let it know your ready for it to start sending chunks

            print(f"Chunk: {chunk}")
            if chunk != prevChunk:
                print("If statement")
                chunk_str = chunk.decode()
                print(chunk_str)
                if chunk_str == "EOF":  # Or another end marker your Arduino sends
                    print("EOF")
                    break
                image_chunks.append(chunk_str)
                prevChunk = chunk
                counter = counter + 1
                print("Updated prevChunk")
                await asyncio.sleep(0.05)  # Give the Arduino time to send next chunk
            else: 
                await asyncio.sleep(0.01)  #testing this john

        image_base64 = ''.join(image_chunks)
        image_data = base64.b64decode(image_base64)

        filename = "output.jpg"
        with open(filename, "wb") as f:
            f.write(image_data)
        print(f"Saved image as {filename}")
        print(counter)


        await client.disconnect() # disconecs to connect later making it ble server
        print(f"disconnect from: {address}")


#10 Seconds to connect to the device

asyncio.run(main())