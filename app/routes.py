from flask import render_template, flash, redirect
from app import app, db
from app.forms import LoginForm, UpdateButton
from app.models import UIDtable
import requests, json, base64

@app.route("/")
@app.route("/index")
def index():
    user = {"username": "Team P&G"}
    return render_template("index.html", title="Home", user=user)


@app.route("/post", methods=["GET", "POST"])
def login():
    form = LoginForm()
    if form.validate_on_submit():
        uid_verify = UIDtable.query.filter_by(UID=form.uid.data).first()
        if uid_verify:
            flash("UID already existing in database")
            return redirect("/index")
        flash("UID requested {}".format(form.uid.data))
        uid = UIDtable(UID=form.uid.data)
        db.session.add(uid)
        db.session.commit()
        return redirect("/index")
    return render_template("post.html", title="Post", form=form)


@app.route("/updategate", methods=["GET", "POST"])
def update_gate():
    button = UpdateButton()
    form = LoginForm()

    if button.validate_on_submit():
        get_uid = UIDtable.query.limit(1).all()
        string = str(get_uid)
        inner = string.split(' ')[len(string.split(' ')) -1 ].split('>')[0]

        message_byte = inner.encode('ascii')
        base64_bytes = base64.b64encode(message_byte)

        parseB = base64_bytes.decode("utf-8")
        parseB = parseB + "==" 

        url = "https://integrations.thethingsnetwork.org/ttn-eu/api/v2/down/my-gate-application/ab?key=ttn-account-v2.xlXa5cnGAmitQlmTxak-euWZ6eZh-Gke4ol-bl6BF6s&"

        payload = json.dumps(
            {
                "dev_id": "my-gate-device",
                "port": 1,
                "confirmed": False,
                "payload_raw": "MTQ5OTMzNjAxNw==",
            }
        )
        headers = {"Content-Type": "application/json"}
        response = requests.request("POST", url, headers=headers, data=payload)
        
    return render_template("updategate.html", title="UpdateGate", form=button)
