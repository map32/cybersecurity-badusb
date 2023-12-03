
# import flast module
from flask import Flask, render_template, request, jsonify
import json
from datetime import datetime

app = Flask(__name__)
def create_timestamp():
    now = datetime.now()
    timestamp = now.strftime('%Y-%m-%d %H:%M:%S')
    return timestamp

# instance of flask application
 
# home route that returns below text when root url is accessed
@app.route("/")
def hello_world():
    return render_template("list.html",logs = data['data'])

@app.route('/post', methods=['GET', 'POST'])
def post():
    if request.method == 'POST':
        elem = request.get_json()
        data['data'].append({'content':elem.get('data',''),'timestamp':create_timestamp(),'name':elem.get('name',''),'ip':elem.get('ip','')})
        with open('data.json', 'w') as f:
            json.dump(data, f)
    else:
        return jsonify({'status': 'give a post request, not get'})
    return jsonify({'status': 'success'})

if __name__ == '__main__':
   with open('data.json','r') as f:
      data = json.load(f)
      print(data)
   app.run(host="0.0.0.0")