import base64
from datetime import datetime

filePath = "C:\\Users\\Bryson Blakney\\Desktop\\base.txt"

file = open(filePath, "r")
base = file.readline()
print(base)
file.close()

imageData = base64.b64decode(base)

current_time_str = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
filename = f"{current_time_str}.jpg"  # Change extension if needed

with open(filename, "wb") as file:
    file.write(imageData)