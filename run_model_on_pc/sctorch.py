import cv2
import numpy as np
import requests
import torch

# Configuration
ESP32_CAM_URL = "http://192.168.178.51/capture"
MODEL_PATH = "maisrecente.tflite"  # Your YOLOv5 PyTorch model

# Load model
model = torch.hub.load('ultralytics/yolov5', 'custom', path=MODEL_PATH)

# Fetch image from ESP32-CAM
def fetch_image():
    try:
        response = requests.get(ESP32_CAM_URL, timeout=5)
        if response.status_code == 200:
            image = cv2.imdecode(np.frombuffer(response.content, np.uint8), cv2.IMREAD_COLOR)
            return image
    except Exception as e:
        print(f"Fetch error: {e}")
    return None

# Main loop
requests.get("http://192.168.178.51/control?var=framesize&val=3")
requests.get("http://192.168.178.51/control?var=led_intensity&val=0")  # ON  # OFF

while True:
    frame = fetch_image()
    if frame is None:
        continue

    # Inference
    results = model(frame)

    # Render results
    annotated_frame = results.render()[0]

    # Show
    cv2.imshow("YOLOv5 Detection", annotated_frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
