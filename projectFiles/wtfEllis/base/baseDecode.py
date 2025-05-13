import base64

filePath = "C:\\Users\\PCHS_BPA\\Downloads\\base.txt"

file = open(filePath, "r")
base = file.readline()
print(base)
file.close()

imageData = base64.b64decode(base)

with open("decoded_image.png", "wb") as file:
    file.write(imageData)