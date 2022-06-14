from app import db
from datetime import datetime
from werkzeug.security import generate_password_hash, check_password_hash
from flask_login import UserMixin
from app import login


@login.user_loader
def load_user(id):
    return User.query.get(int(id))


class User(UserMixin, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(64), index=True, unique=True)
    name = db.Column(db.String(32), index=True)
    password_hash = db.Column(db.String(128))
    borrows = db.relationship('Borrow', backref='user_id', lazy='dynamic')

    def __repr__(self):
        return '<User {}>'.format(self.username)

    def set_password(self, password):
        self.password_hash = generate_password_hash(password)

    def check_password(self, password):
        return check_password_hash(self.password_hash, password)


class Bookinfo(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(128), index=True)
    author = db.Column(db.String(128), index=True)
    publisher = db.Column(db.String(64))
    introduction = db.Column(db.String(512))
    books = db.relationship('Book', backref='bookinfo_id', lazy='dynamic')

    def __repr__(self):
        return '<Book {}>'.format(self.name)


class Book(db.Model):
    barcode = db.Column(db.String(128), primary_key=True)
    status = db.Column(db.String(32))
    position = db.Column(db.String(32))
    id_bookinfo = db.Column(db.Integer, db.ForeignKey('bookinfo.id'))
    borrow = db.relationship('Borrow', backref='borrow_book', lazy='dynamic')

    def __repr__(self):
        return '<Bookcode {}>'.format(self.barcode)


class Borrow(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    barcode_book = db.Column(db.String(128), db.ForeignKey('book.barcode'))
    id_use = db.Column(db.Integer, db.ForeignKey('user.id'))
    start = db.Column(db.String(32), index=True)
    end = db.Column(db.String(32), index=True)
    status = db.Column(db.String(32), index=True)


class Librarian(UserMixin, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(64), index=True, unique=True)
    name = db.Column(db.String(32), index=True)
    password_hash = db.Column(db.String(128))

    def set_password(self, password):
        self.password_hash = generate_password_hash(password)

    def check_password(self, password):
        return check_password_hash(self.password_hash, password)


class Admin(UserMixin, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(64), index=True, unique=True)
    password_hash = db.Column(db.String(128))

    def check_password(self, password):
        return check_password_hash(self.password_hash, password)

