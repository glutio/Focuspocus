# Turns the frames from "focuspocus_screenshots.exe gif" (build/gif/*.png) into
# examples/Demo/demo.gif, with a red ring following the finger (thicker while pressed).
# Run from the repository root; needs Pillow.
import glob
from PIL import Image, ImageDraw

touches = open("build/gif/touches.txt").read().split("\n")
frames = []
for i, f in enumerate(sorted(glob.glob("build/gif/*.png"))):
    img = Image.open(f).convert("RGBA")
    if i < len(touches) and touches[i]:
        s = img.width / 240
        x, y, down = (int(v) for v in touches[i].split())
        x, y, r = x * s, y * s, 7 * s
        dot = Image.new("RGBA", img.size)
        ImageDraw.Draw(dot).ellipse((x - r, y - r, x + r, y + r), outline=(255, 0, 0, 255),
                                    width=int((3 if down else 1.5) * s))
        img = Image.alpha_composite(img, dot)
    frames.append(img.convert("RGB"))
frames[0].save("examples/Demo/demo.gif", save_all=True, append_images=frames[1:],
               duration=60, loop=0, optimize=True)
print(len(frames), "frames")
