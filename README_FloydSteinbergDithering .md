Dithering is an algorithmic technique used when you need to display or store an image with **fewer colors (or lower bit depth)** than it originally has. Instead of just rounding each pixel to the nearest available color (which causes banding and harsh transitions), dithering introduces a pattern of pixel-level variations that **trick the human eye into perceiving smoother gradients and intermediate tones**.

---

## Key Idea

Human vision integrates small patterns of light and color. By mixing pixels of nearby available colors, dithering simulates colors or shades that aren’t directly present in the palette.

For example, if you can only display black and white:

* A 50% gray pixel is simulated by alternating black and white pixels in a checkerboard pattern.

---

## Types of Dithering Algorithms

### 1. **Ordered Dithering**

* Uses a fixed threshold matrix (e.g., Bayer matrix).
* Each pixel’s intensity is compared against the threshold to decide if it’s mapped up or down.
* Produces a regular, patterned texture.
* Fast, but sometimes creates visible repeating grids.

**Example:**

```text
Bayer 2×2 matrix:
[ 0  2 ]
[ 3  1 ]
```

Pixel values are compared against a scaled version of this matrix to decide black/white.

---

### 2. **Error Diffusion Dithering**

* Processes the image pixel by pixel, quantizing each to the nearest available color.
* The *error* (difference between the original and quantized value) is spread to neighboring pixels, so the total brightness is preserved locally.
* Produces a more natural, random-looking texture.

**Popular methods:**

* **Floyd–Steinberg** (most famous):

  ```text
      *   7/16
  3/16  5/16  1/16
  ```

  The quantization error is distributed to right and bottom neighbors.

# Floyd–Steinberg dithering — step-by-step (4×4 example)

We'll dither this **4×4 grayscale** image to **black & white** using **Floyd–Steinberg** (threshold = 128).

**Original image (values 0–255):**

```
[ [ 34,  67, 190, 255]
  [120, 140, 200, 210]
  [ 80,  90, 160, 170]
  [ 30,  60, 120, 130] ]
```

**Weights (error distribution):**
right = 7/16, bottom-left = 3/16, bottom = 5/16, bottom-right = 1/16.

We scan left → right, top → bottom. For each pixel:

* `new = 255` if `old >= 128`, otherwise `new = 0`
* `error = old - new` is distributed to neighbors using the weights above.

---

### Detailed per-pixel steps (row-major order)

Pixel (0,0): old = 34.000 → new = 0, error = 34.000

* add 14.875 to pixel (1,0) → becomes 81.875
* add 10.625 to pixel (0,1) → becomes 130.625
* add 2.125 to pixel (1,1) → becomes 142.125

Pixel (1,0): old = 81.875 → new = 0, error = 81.875

* add 35.820 to pixel (2,0) → becomes 225.820
* add 15.352 to pixel (0,1) → becomes 145.977
* add 25.586 to pixel (1,1) → becomes 167.711
* add 5.117 to pixel (2,1) → becomes 205.117

Pixel (2,0): old = 225.820 → new = 255, error = -29.180

* add -12.766 to pixel (3,0) → becomes 242.234
* add -5.471 to pixel (1,1) → becomes 162.240
* add -9.119 to pixel (2,1) → becomes 195.999
* add -1.824 to pixel (3,1) → becomes 208.176

Pixel (3,0): old = 242.234 → new = 255, error = -12.766

* (right neighbor out of bounds)
* add -2.398 to pixel (2,1) → becomes 193.600
* add -3.992 to pixel (3,1) → becomes 204.184
* (bottom-right out of bounds)

Pixel (0,1): old = 145.977 → new = 255, error = -109.023

* add -47.662 to pixel (1,1) → becomes 119.449
* add -20.441 to pixel (0,2) → becomes 100.441
* add -34.069 to pixel (1,2) → becomes 124.069

Pixel (1,1): old = 119.449 → new = 0, error = 119.449

* add 52.246 to pixel (2,1) → becomes 245.846
* add 22.409 to pixel (0,2) → becomes 122.850
* add 37.328 to pixel (1,2) → becomes 161.397
* add 7.466 to pixel (2,2) → becomes 167.466

Pixel (2,1): old = 245.846 → new = 255, error = -9.154

* add -4.004 to pixel (3,1) → becomes 200.180
* add -1.717 to pixel (1,2) → becomes 159.680
* add -2.860 to pixel (2,2) → becomes 164.606
* add -0.572 to pixel (3,2) → becomes 169.428

Pixel (3,1): old = 200.180 → new = 255, error = -54.820

* (right out of bounds)
* add -10.297 to pixel (2,2) → becomes 154.309
* add -17.131 to pixel (3,2) → becomes 152.297
* (bottom-right out of bounds)

Pixel (0,2): old = 122.850 → new = 0, error = 122.850

* add 53.856 to pixel (1,2) → becomes 213.536
* add 23.019 to pixel (0,3) → becomes 53.019
* add 7.678 to pixel (1,3) → becomes 67.678

Pixel (1,2): old = 213.536 → new = 255, error = -41.464

* add -18.098 to pixel (2,2) → becomes 136.211
* add -7.773 to pixel (0,3) → becomes 45.246
* add -12.966 to pixel (1,3) → becomes 54.711
* add -2.592 to pixel (2,3) → becomes 157.408

Pixel (2,2): old = 136.211 → new = 255, error = -118.789

* add -51.915 to pixel (3,2) → becomes 100.382
* add -22.348 to pixel (1,3) → becomes 32.363
* add -37.121 to pixel (2,3) → becomes 120.287
* add -7.424 to pixel (3,3) → becomes 122.576

Pixel (3,2): old = 100.382 → new = 0, error = 100.382

* (right out of bounds)
* add 42.144 to pixel (2,3) → becomes 162.431
* add 31.369 to pixel (3,3) → becomes 153.945
* (bottom-right out of bounds)

Pixel (0,3): old = 45.246 → new = 0, error = 45.246

* add 19.792 to pixel (1,3) → becomes 52.155
* (no bottom row to distribute further)

Pixel (1,3): old = 52.155 → new = 0, error = 52.155

* add 22.795 to pixel (2,3) → becomes 185.226

Pixel (2,3): old = 185.226 → new = 255, error = -69.774

* add -30.452 to pixel (3,3) → becomes 123.493

Pixel (3,3): old = 123.493 → new = 0, error = 123.493

* no neighbors to distribute to (edge)

---

**Final dithered image (0 = black, 255 = white):**

```
[ [   0,   0, 255, 255 ]
  [ 255,   0, 255, 255 ]
  [   0, 255,   0, 255 ]
  [   0,   0, 255,   0 ] ]
```

---

That’s the full numeric walkthrough: for each pixel we show the original (possibly already updated by previous errors), the quantized result (0 or 255), the error, and how that error is split to neighbors.

Would you like:

* a short Python script that performs this on any grayscale image and prints similar step-by-step logs, or
* a visualization (image) showing the original and dithered result?
