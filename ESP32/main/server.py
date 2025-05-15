from flask import Flask, request

app = Flask(__name__)

@app.route('/upload', methods=['POST'])
def upload():
    data = request.data
    with open("imagem_recebida.jpg", "wb") as f:
        f.write(data)
    print("Imagem recebida!")
    return "OK", 200

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=80)