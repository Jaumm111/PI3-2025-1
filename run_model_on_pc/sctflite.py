import cv2
import numpy as np
import requests
import tensorflow as tf

# Configuration
ESP32_CAM_URL = "http://192.168.178.51/capture"
MODEL_PATH = "maisrecente.tflite"
LABELS_PATH = "labels.txt"

# Load labels
try:
    with open(LABELS_PATH, 'r') as f:
        labels = [line.strip() for line in f.readlines()]
except FileNotFoundError:
    print(f"Error: {LABELS_PATH} not found")
    exit()

# Load TFLite model
interpreter = tf.lite.Interpreter(model_path=MODEL_PATH)
#interpreter = tf.lite.Interpreter(model_path=MODEL_PATH, experimental_delegates=[])

interpreter.allocate_tensors()
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()
height, width = input_details[0]['shape'][1:3]
is_quantized = input_details[0]['dtype'] == np.int8  # FP16 uses float32

print("Input details:", input_details)


# Preprocess image
def preprocess_image(image):
    image = cv2.resize(image, (width, height))
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    image = image.astype(np.float32) / 255.0  # FP16 expects float32, 0-1
    return np.expand_dims(image, axis=0)

# Fetch image from ESP32-CAM
def fetch_image():
    try:
        response = requests.get(ESP32_CAM_URL, timeout=5)
        if response.status_code == 200:
            return cv2.imdecode(np.frombuffer(response.content, np.uint8), cv2.IMREAD_COLOR)
        print(f"HTTP {response.status_code}")
        return None
    except Exception as e:
        print(f"Error: {e}")
        return None

# Run inference
def run_inference(image):
    input_data = preprocess_image(image)
    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()
    output = interpreter.get_tensor(output_details[0]['index'])[0]  # Single output tensor
    boxes = output[..., :4]  # [num_boxes, 4] (center_x, center_y, width, height)
    scores = output[..., 4]  # [num_boxes]
    classes = np.argmax(output[..., 5:], axis=-1)  # [num_boxes]
    return boxes, classes, scores

# Draw bounding boxes
def draw_boxes(image, boxes, classes, scores, threshold=0.95):
    h, w, _ = image.shape
    for i in range(len(scores)):
        if scores[i] > threshold:
            cx, cy, bw, bh = boxes[i]
            xmin, xmax = int((cx - bw / 2) * w), int((cx + bw / 2) * w)
            ymin, ymax = int((cy - bh / 2) * h), int((cy + bh / 2) * h)
            cv2.rectangle(image, (xmin, ymin), (xmax, ymax), (0, 255, 0), 2)
            label = f"{labels[int(classes[i])]}: {scores[i]:.2f}"
            cv2.putText(image, label, (xmin, ymin - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
    return image

# Main loop
requests.get("http://192.168.178.51/control?var=framesize&val=3")
requests.get("http://192.168.178.51/control?var=led_intensity&val=0")  # ON  # OFF

while True:
    image = fetch_image()
    if image is None:
        continue
    boxes, classes, scores = run_inference(image)
    image_with_boxes = draw_boxes(image, boxes, classes, scores)
    cv2.imshow("Object Detection", image_with_boxes)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()