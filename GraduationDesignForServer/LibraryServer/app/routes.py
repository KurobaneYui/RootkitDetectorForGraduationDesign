from flask import render_template, flash, redirect, url_for, request
from app import app
from flask_login import current_user, login_user
from app.models import User
from flask_login import logout_user, login_required
from app.forms import LoginForm
from werkzeug.urls import url_parse


@app.route('/logout')
def logout():
    logout_user()
    return redirect(url_for('login'))


@app.route('/login.html', methods=['GET', 'POST'])
def login():

    return render_template('login.html')


@app.route('/')
@app.route('/RootkitInfoCenter.html')
def searchBook():

    return render_template('RootkitInfoCenter.html')


@app.route('/userProfile.html')
@login_required
def userProfile():

    return render_template('userProfile.html')


@app.route('/borrowAndReturn.html')
@login_required
def borrowAndReturn():

    return render_template('borrowAndReturn.html')


@app.route('/manageBooks.html')
@login_required
def manageBooks():

    return render_template('manageBooks.html')


@app.route('/administrator.html')
@login_required
def admin():

    return render_template('administrator.html')