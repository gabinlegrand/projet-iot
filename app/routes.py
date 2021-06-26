from flask import render_template, flash, redirect, request
from app import app, db
from app.forms import LoginForm, RegistrationForm, UpdateButton, OpenDoor, CloseDoor, LoginConnect, RegistrationForm
from app.models import UIDtable, User
import requests, json, base64, time
from flask_login import current_user, login_user, logout_user, login_required
from flask_material import Material
import time
from datetime import datetime

Material(app)
# Home Page
@app.route("/")
@app.route("/index")
def index():
    return render_template("index.html", title="Home")

# Login Page
@app.route('/login', methods=['GET', 'POST'])
def loginuser():
    if current_user.is_authenticated:
        return redirect('/index')
    form = LoginConnect()
    
    if form.validate_on_submit():
        # Check user data
        user = User.query.filter_by(username=form.username.data).first()
        if user is None or not user.check_password(form.password.data):
            flash('Invalid username or password')
            return redirect('/login')
        login_user(user, remember=form.remember_me.data)
        return redirect("/index")
    
    return render_template('login.html', title='Sign In', form=form)

# Disconnect current user
@app.route('/logout')
def logout():
    logout_user()
    return redirect("/index")

# Register new user
@app.route('/register', methods=['GET', 'POST'])
def register():
    if current_user.is_authenticated:
        return redirect("/index")
    
    form = RegistrationForm()
    
    if form.validate_on_submit():
        user = User(username=form.username.data, email=form.email.data)
        user.set_password(form.password.data)
        db.session.add(user)
        db.session.commit()
        flash('Congratulations, you are now a registered user!')
        return redirect("/login")
    
    return render_template('register.html', title='Register', form=form)

# Add UID in db
@app.route("/post", methods=["GET", "POST"])
@login_required
def login():
    form = LoginForm()
    if form.validate_on_submit():
        uid_verify = UIDtable.query.filter_by(UID=form.uid.data).first()
        
        # Duplicate Verification
        if uid_verify:
            flash("UID already existing in database")
            return redirect("/index")
        flash("UID requested {}".format(form.uid.data))
        uid = UIDtable(UID=form.uid.data)
        db.session.add(uid)
        db.session.commit()
        return redirect("/index")
    
    return render_template("post.html", title="Post", form=form)

# Close door
def close_gate():
    url = "https://integrations.thethingsnetwork.org/ttn-eu/api/v2/down/my-gate-application/ab?key=ttn-account-v2.xlXa5cnGAmitQlmTxak-euWZ6eZh-Gke4ol-bl6BF6s&"
    payload = json.dumps(
        {
            "dev_id": "my-gate-device",
            "port": 1,
            "confirmed": False,
            "payload_raw": "Y2xvc2U=",
        }
    )
    headers = {"Content-Type": "application/json"}
    response = requests.request("POST", url, headers=headers, data=payload)

# Update UID List on Arduino
@app.route("/updategate", methods=["GET", "POST"])
@login_required
def update_gate():
    button = UpdateButton()

    if button.validate_on_submit():
        # Get UID db list
        get_uid = UIDtable.query.limit(50).all()
        
        # Loop Convert in b64 -> Downlink
        for uid_n in get_uid:
            string = str(uid_n)
            inner = string.split(' ')[len(string.split(' ')) -1 ].split('>')[0]

            message_byte = inner.encode('ascii')
            base64_bytes = base64.b64encode(message_byte)

            parseB = base64_bytes.decode("utf-8")

            url = "https://integrations.thethingsnetwork.org/ttn-eu/api/v2/down/my-gate-application/ab?key=ttn-account-v2.xlXa5cnGAmitQlmTxak-euWZ6eZh-Gke4ol-bl6BF6s&"
            payload = json.dumps(
                {
                    "dev_id": "my-gate-device",
                    "port": 1,
                    "confirmed": False,
                    "payload_raw": parseB,
                }
            )
            headers = {"Content-Type": "application/json"}
            response = requests.request("POST", url, headers=headers, data=payload)
            flash(response.status_code)
            time.sleep(5)
        
    return render_template("updategate.html", title="UpdateGate", form=button)

# Open Door
@app.route("/opengate", methods=["GET", "POST"])
@login_required
def open_gate():
    opendoor = OpenDoor()
    closedoor = CloseDoor()

    if "open-submit" in request.form and opendoor.submitOpen.validate(opendoor):
        url = "https://integrations.thethingsnetwork.org/ttn-eu/api/v2/down/my-gate-application/ab?key=ttn-account-v2.xlXa5cnGAmitQlmTxak-euWZ6eZh-Gke4ol-bl6BF6s&"
        payload = json.dumps(
            {
                "dev_id": "my-gate-device",
                "port": 1,
                "confirmed": False,
                "payload_raw": "b3Blbg==",
            }
        )
        headers = {"Content-Type": "application/json"}
        response = requests.request("POST", url, headers=headers, data=payload)
        flash(response.status_code)
    elif "close-submit" in request.form and closedoor.submitClose.validate(closedoor):
        close_gate()
    return render_template("opengate.html", title="OpenGate", form1=opendoor,form2=closedoor)

# Custom task
@app.route("/scheduled_task", methods=["GET", "POST"])
def scheduled_task():
    print("Working")
    url = "http://api.openweathermap.org/data/2.5/weather?q=amiens&appid=429720d2f79e1dc6186c62df2374635b"

    payload = ""
    headers = {}

    response = requests.request("GET", url, headers=headers, data=payload)
    result = response.json()

    wind = result["wind"]["speed"]

    if wind > 20: #Close submit when wind is too strong
        close_gate()
    else:
        print("Wind OK")
        
    hour = 14
    minute = 50
    if hour == datetime.now().hour and minute == datetime.now().minute:
        print("C'est l'heure")
        close_gate()
    else:
        print("C'est pas l'heure")
    return "OK", 200
