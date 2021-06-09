from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, BooleanField, SubmitField
from wtforms.validators import DataRequired

class LoginForm(FlaskForm):
    uid = StringField('UID', validators=[DataRequired()])
    submit = SubmitField('Send')

class UpdateButton(FlaskForm):
    submit = SubmitField('Request upate')