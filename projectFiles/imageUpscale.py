from PIL import Image # type: ignore

img = Image.open("output.jpg")
width, height = img.size
upscaled_img = img.resize((width * 2, height * 2), Image.Resampling.LANCZOS)
upscaled_img.save("output_pil.jpg")