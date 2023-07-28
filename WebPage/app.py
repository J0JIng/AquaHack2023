from flask import Flask, Response, render_template, request, jsonify, session
import sys
import requests
import time

# In the future to scale up the project, we can use a database to store all the ESP32-CAM IP addresses
# each users can only see one stream in each instance, but they can switch between streams
# to do so the ESP32_IP would be sent via the POST request from the frontend
ESP32_IP = "192.168.43.153"; # Enter the IP address of your ESP32-CAM here
# Create the Flask object for the application
app = Flask(__name__)

app.secret_key = 'super secret key'
# Global variable to store the video frame
video_frame = None

# Queue to store the names of people who press the button
name_submitted = set();
# Route to fetch the names from the queue and send them to the webpage
@app.route('/get_names')
def get_names():
    print(name_submitted, file=sys.stderr)
    return jsonify(list(name_submitted))

# Route to receive the video stream from the ESP32-CAM
# (keep your existing /receive_video route as it is)
@app.route('/receive_video', methods=['POST'])
def receive_video():
    global video_frame
    video_frame = request.data
    return "Received video frame"

# Route to serve the webpage with SSE connection for video streaming
@app.route('/')
def index():
    return render_template('index.html')

# Route to serve the video stream to the SSE connection
@app.route('/stream_video')
def stream_video():
    def generate():
        while True:
            if video_frame:
                yield (b'--frame\r\n' b'Content-Type: application/octet-stream\r\n\r\n' + video_frame + b'\r\n\r\n')
            else:
                yield (b'--frame\r\n' b'Content-Type: application/octet-stream\r\n\r\n' + b'\r\n\r\n')

    return Response(generate(), mimetype='multipart/x-mixed-replace; boundary=frame')

# Route to handle button press and store names in the queue
@app.route('/button_press', methods=['POST'])
def button_press():
    #Ew too many response and if...else statements
    name = request.json['name']
    print(request.json, file=sys.stderr)
    #print(f'Name: {name}', file=sys.stderr)
    if name not in name_submitted:
        if len(name_submitted) > 9:
            return "Name limit reached"
        if name in name_submitted:
            return "Name already submitted"
        session['name'] = name
        name_submitted.add(name)
        return "Name added to the queue"
    else:
        return "No name provided"
    
# Route to handle the "feed" button press from the frontend
@app.route('/feed', methods=['POST'])
def feed():
    response = requests.post(f'http://{ESP32_IP}/feed') # Send a POST request to the ESP32-CAM
    print("Status:" , response.status_code, file=sys.stderr)
    if 'name' in session:
        # Allow the user to press the "feed" button since their name is in the session
        response = requests.post(f'http://{ESP32_IP}/feed') # Send a POST request to the ESP32-CAM
        print("Status:" , response.status_code, file=sys.stderr)
        if response.status_code != 200:
            print(name_submitted, file=sys.stderr)
            #clear_session();
            return "Feeding the fish"
        else:

            print(name_submitted, file=sys.stderr)
            return "Error feeding the fish"
    else:
        print(name_submitted, file=sys.stderr)
        return "You need to submit your name first"



def clear_session():
    time.sleep(60)
    session.pop('name', None)
    name_submitted.clear()

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
