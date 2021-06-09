import os
basedir = os.path.abspath(os.path.dirname(__file__))

class Config(object):
    SECRET_KEY = os.environ.get('SECRET_KEY') or 'you-will-never-guess'

    SQLALCHEMY_DATABASE_URI = os.environ.get ('mimolette.fr') or \
        'mssql+pymssql://webServer:projetIoT1234@mimolette.fr:1433/projetIOT'
    SQLALCHEMY_TRACK_MODIFICATIONS = False
