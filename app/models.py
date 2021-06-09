from app import db

class UIDtable(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    UID = db.Column(db.String(20), index=True, unique=True)

    def __repr__(self):
        return 'UID {}>'.format(self.UID)

