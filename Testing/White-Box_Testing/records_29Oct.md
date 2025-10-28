| Area                   | Method                                | Key Observations                                                   |
| ---------------------- | ------------------------------------- | ------------------------------------------------------------------ |
| Distance calculation   | Serial monitor output                 | Verified distance updates ~30ms per loop                           |
| Speed computation      | Compared distance difference/time     | Detected accurate direction (positive = away, negative = approach) |
| Brightness mapping     | Reviewed value scaling (map function) | Correctly constrained between 30–100% brightness                   |
| Color mapping function | Tested distance values 5–50cm         | Gradient follows Red → Orange → Yellow → Green → Blue sequence     |
| MQTT payload size      | Measured payload bytes                | < 2000 bytes; stable for 72 LEDs                                   |
