from flask import render_template, flash, redirect, url_for, request, jsonify
from app import app, db
from flask_login import current_user, login_user
from app.models import User, Borrow, Book, Bookinfo, Librarian, Admin
from flask_login import logout_user, login_required
from werkzeug.urls import url_parse
from werkzeug.security import generate_password_hash, check_password_hash
import datetime
import sqlite3


# 0-1.用户设备获取
@app.route('/func/UserDevices', methods=['POST'])
def User_Devices():
    data = request.get_json()
    UserID = int(data["UserID"])
    
    database = sqlite3.connect("/home/ubuntu/GraduationDesign/Graduation.db")
    cursor = database.cursor()
    sql = "select DeviceID from DeviceTable where UserID=?"
    cursor.execute(sql,(UserID,))
    DeviceIDs_tmp = cursor.fetchall()
    DeviceIDs = list()
    for i in DeviceIDs_tmp:
        DeviceIDs.append(i[0])
    
    returns = {'return_code': 200}
    returns['DeviceIDs'] = DeviceIDs
    
    cursor.close()
    database.close()
    
    return jsonify(returns)

# 0-2.设备进程信息获取
@app.route('/func/DeviceRootkits', methods=['POST'])
def Device_Rootkits():
    data = request.get_json()
    UserID = data["UserID"]
    DeviceID = data["DeviceID"]
    
    database = sqlite3.connect("/home/ubuntu/GraduationDesign/Graduation.db")
    cursor = database.cursor()
    sql = "select DID  from DeviceTable where UserID=? and DeviceID=?"
    cursor.execute(sql,(UserID,DeviceID))
    DID = cursor.fetchall()
    if len(DID)==0:
        return jsonify({'return_code': 0})
    DID = DID[0][0]
    sql = "select * from ProcessInfoTable where DID=? order by InsertDate desc limit 30"
    cursor.execute(sql,(DID,))
    RootkitsInfos_tmp = cursor.fetchall()
    RootkitsInfos = list()
    for i in RootkitsInfos_tmp:
        RootkitsInfos.append({'pid':i[2],'parentPid':i[3],'shortName':i[4],'createPath':i[5].decode("utf-8"),'datetime':i[6]})
    
    returns = {'return_code':200}
    returns['ProcessInfos'] = RootkitsInfos
    
    return jsonify(returns)


# 1.用户登录
@app.route('/func/login', methods=['POST'])
def user_login():
    data = request.get_json()
    username = data['username']
    if data['type'] == "0":
        user = User.query.filter_by(username=username).first()
        if user is None:
            return jsonify({'return_code': 301})    #找不到用户
        password = data['password']
        if not user.check_password(password):
            return jsonify({'return_code': 302})    #密码错误
        login_user(user)
        data = jsonify({'return_code': 200,
                        'id': user.id,
                        'username': user.username,
                        'name': user.name})
        return data
    if data['type'] == "1":
        librarian = Librarian.query.filter_by(username=username).first()
        if librarian is None:
            return jsonify({'return_code': 301})
        password = data['password']
        if not librarian.check_password(password):
            return jsonify({'return_code': 302})
        login_user(librarian)
        data = jsonify({'return_code': 200,
                        'id': librarian.id,
                        'username': librarian.username,
                        'name': librarian.name})
        return data
    if data['type'] == "2":
        admin = Admin.query.filter_by(username=username).first()
        if admin is None:
            jsonify({'return_code': 301})
        password = data['password']
        if not admin.check_password(password):
            return jsonify({'return_code': 302})
        login_user(admin)
        data = jsonify({'return_code': 200,
                        'id': admin.id,
                        'username': admin.username})
        return data


# 2.注销
@app.route('/func/logout', methods=['POST'])
def user_logout():
    logout_user()
    return jsonify({'return_code': 200})


# 3.搜索
@app.route('/func/search', methods=['POST'])
def search():
    data = request.get_json()
    purpose = data['purpose']
    if purpose == '0':
        type = data['type']
        if type == '0':
            bookinfo = Bookinfo.query.filter(Bookinfo.name.like("%" + data['keywords'] + "%")).all()[0:9]
            if not bookinfo:
                return jsonify({'return_code': 301})
            bookinfolist = []
            for i in bookinfo:
                temp_dict = {}
                temp_dict['id'] = i.id
                temp_dict['name'] = i.name
                temp_dict['author'] = i.author
                temp_dict['publisher'] = i.publisher
                temp_dict['introduction'] = i.introduction
                temp_dict['num'] = i.books.count()
                bookinfolist.append(temp_dict)
            return jsonify({'return_code': 200,
                            'resultnum': len(bookinfolist),
                            'result': bookinfolist
                            })
        if type == '1':
            bookinfo = Bookinfo.query.filter(Bookinfo.author.like("%" + data['keywords'] + "%")).all()[0:9]
            if not bookinfo:
                return jsonify({'return_code': 301})
            bookinfolist = []
            for i in bookinfo:
                temp_dict = {}
                temp_dict['id'] = i.id
                temp_dict['name'] = i.name
                temp_dict['author'] = i.author
                temp_dict['publisher'] = i.publisher
                temp_dict['introduction'] = i.introduction
                temp_dict['num'] = len(i.books)
                bookinfolist.append(temp_dict)
            return jsonify({'return_code': 200,
                            'resultnum': len(bookinfolist),
                            'result': bookinfolist
                            })

    if purpose == '1':
        user = User.query.filter(User.id.like("%" + data['keywords'] + "%")).all()[0:9]
        if not user:
            return jsonify({'return_code': 301})
        userlist = []
        for i in user:
            temp_dict = {}
            temp_dict['id'] = i.id
            temp_dict['username'] = i.username
            temp_dict['name'] = i.name
            userlist.append(temp_dict)
        return jsonify({'return_code': 200,
                        'resultnum': len(userlist),
                        'result': userlist
                        })

    if purpose == '2':
        librarian = Librarian.query.filter(Librarian.name.like("%" + data['keywords'] + "%")).all()[0:9]
        if not librarian:
            return jsonify({'return_code': 301})
        librarianlist = []
        for i in librarian:
            temp_dict = {}
            temp_dict['id'] = i.id
            temp_dict['username'] = i.username
            temp_dict['name'] = i.name
            librarianlist.append(temp_dict)
        return jsonify({'return_code': 200,
                        'resultnum': len(librarianlist),
                        'result': librarianlist
                        })
    return {}


# 4.获取用户信息
@app.route('/func/user/profile', methods=['POST'])
@login_required
def get_profile():
    user = current_user
    if user is None:
        return jsonify({'return_code': 301})
    borrows = user.borrows
    borrowlist = []
    today = datetime.datetime.today()
    for i in borrows:
        temp_dict = {}
        temp_dict['bookname'] = i.borrow_book.bookinfo_id.name
        temp_dict['start'] = i.start
        temp_dict['end'] = i.end
        end = datetime.datetime.strptime(i.end, '%Y-%m-%d')
        if end - today < datetime.timedelta(days=1):
            i.status = '逾期'
        temp_dict['status'] = i.status
        borrowlist.append(temp_dict)
    db.session.commit()
    data = jsonify({'return_code': 200,
                    'id': user.id,
                    'username': user.username,
                    'name': user.name,
                    'borrownum': len(borrowlist),
                    'borrows': borrowlist})
    return data


# 5.获取书籍信息
@app.route('/func/bookinfo/get', methods=['POST'])
def get_bookinfo():
    data = request.get_json()
    id = data['id']
    bookinfo = Bookinfo.query.filter_by(id=id).first()
    if not bookinfo:
        return jsonify({'return_code': 301})
    return jsonify({'return_code': 200,
                    'id': bookinfo.id,
                    'name': bookinfo.name,
                    'author': bookinfo.author,
                    'publisher': bookinfo.publisher,
                    'introduction': bookinfo.introduction})


# 6.获取书本信息
@app.route('/func/book/get', methods=['POST'])
def get_books():
    data = request.get_json()
    id = data['id']
    bookinfo = Bookinfo.query.filter_by(id=id).first()
    if not bookinfo:
        return jsonify({'return_code': 301})
    books = bookinfo.books
    booklist = []
    for i in books:
        temp_dict = {}
        temp_dict['barcode'] = i.barcode
        temp_dict['status'] = i.status
        temp_dict['position'] = i.position
        booklist.append(temp_dict)
    return jsonify({'return_code': 200,
             'booknum': len(booklist),
             'books': booklist})


# 7. 更新书籍信息
@app.route('/func/bookinfo/update', methods=['POST'])
@login_required
def update_bookinfo():
    data = request.get_json()
    bookinfo = Bookinfo.query.filter_by(id=data['id']).first()
    if not bookinfo:
        return jsonify({'return_code': 301})
    bookinfo.name = data['name']
    bookinfo.author = data['author']
    bookinfo.publisher = data['publisher']
    bookinfo.introduction = data['introduction']
    db.session.commit()
    return jsonify({'return_code': 200})


# 8. 增加书籍信息
@app.route('/func/bookinfo/add', methods=['POST'])
@login_required
def add_bookinfo():
    data = request.get_json()
    bookinfo = Bookinfo(name=data['name'],
                        author=data['author'],
                        publisher=data['publisher'],
                        introduction=data['introduction'])
    db.session.add(bookinfo)
    db.session.commit()
    return jsonify({'return_code': 200,
                    'id': bookinfo.id})


# 9. 删除书籍信息
@app.route('/func/bookinfo/delete', methods=['POST'])
@login_required
def delete_bookinfo():
    data = request.get_json()
    bookinfo = Bookinfo.query.filter_by(id=data['id']).first()
    if not bookinfo:
        return ({'return_code': 301})
    for i in bookinfo.books:
        db.session.delete(i)
    db.session.delete(bookinfo)
    db.session.commit()
    return jsonify({'return_code': 200})


# 10. 增加书本
@app.route('/func/book/add', methods=['POST'])
@login_required
def add_books():
    data = request.get_json()
    book_test = Book.query.filter_by(barcode=data['barcode']).first()
    if book_test:
        return jsonify({'return_code': 301})
    book = Book(id_bookinfo=data['id'],
                barcode=data['barcode'],
                status=data['status'],
                position=data['position'])
    db.session.add(book)
    db.session.commit()
    return jsonify({'return_code': 200})


# 11. 删除书本
@app.route('/func/book/delete', methods=['POST'])
@login_required
def delete_books():
    data = request.get_json()
    book = Book.query.filter_by(barcode=data['barcode']).first()
    if not book:
        return jsonify({'return_code': 301})
    db.session.delete(book)
    db.session.commit()
    return jsonify({'return_code': 200})


# 12. 更新书本
@app.route('/func/book/update', methods=['POST'])
@login_required
def update_books():
    data = request.get_json()
    book = Book.query.filter_by(barcode=data['barcode']).first()
    if not book:
        return jsonify({'return_code': 301})
    book.id_bookinfo = data['id']
    book.status = data['status']
    book.position = data['position']
    db.session.commit()
    return jsonify({'return_code': 200})


# 13. 借阅
@app.route('/func/borrow', methods=['POST'])
@login_required
def borrow():
    data = request.get_json()
    book = Book.query.filter_by(barcode=data['barcode']).first()
    if not book:
        return jsonify({'return_code': 301})
    user = User.query.filter_by(id=data['id']).first()
    if not user:
        return jsonify({'return_code': 301})
    today = datetime.date.today()
    end = today + datetime.timedelta(days=15)
    borrow = Borrow(barcode_book=data['barcode'],
                    id_use=data['id'],
                    start=today.isoformat(),
                    end=end.isoformat(),
                    status="在借")
    db.session.add(borrow)
    db.session.commit()
    return jsonify({'return_code': 200})


# 14. 归还
@app.route('/func/return', methods=['POST'])
@login_required
def book_return():
    data = request.get_json()
    borrow = Borrow.query.filter_by(barcode_book=data['barcode'],
                                    id_use=data['id'], status="在借").first()
    if not borrow:
        return jsonify({'return_code': 301})
    borrow.status = "归还"
    db.session.commit()
    return jsonify({'return_code': 200})


# 15. 获取图书管理员信息
@app.route('/func/librarian/profile', methods=['POST'])
@login_required
def get_librarian():
    data = request.get_json()
    librarian = Librarian.query.filter_by(id=data['id']).first()
    if not librarian:
        return jsonify({'return_code': 301})
    return jsonify({'return_code': 200,
                    'name': librarian.name})


# 16. 增加用户
@app.route('/func/user/add', methods=['POST'])
@login_required
def add_user():
    data = request.get_json()
    user = User(id=int(data['id']),
                username=data['username'],
                name=data['name'],
                password_hash=generate_password_hash(data['password']))
    db.session.add(user)
    db.session.commit()
    return jsonify({'return_code': 200})


# 17. 删除用户
@app.route('/func/user/delete', methods=['POST'])
@login_required
def delete_user():
    data = request.get_json()
    user = User.query.filter_by(id=data['id']).first()
    if not user:
        return jsonify({'return_code': 301})
    db.session.delete(user)
    db.session.commit()
    return jsonify({'return_code': 200})


# 18. 更改用户密码
@app.route('/func/user/pwdchange', methods=['POST'])
@login_required
def pwdchange_user():
    data = request.get_json()
    user = User.query.filter_by(id=data['id']).first()
    if not user.check_password(data['pwd']):
        return jsonify({'return_code': 302})
    user.set_password(data['pwd_new'])
    db.session.commit()
    return jsonify({'return_code': 200})


# 19. 增加图书管理员
@app.route('/func/librarian/add', methods=['POST'])
@login_required
def add_librarian():
    data = request.get_json()
    librarian = Librarian(username=data['username'],
                          name=data['name'],
                          password_hash=generate_password_hash(data['password']))
    db.session.add(librarian)
    db.session.commit()
    return jsonify({'return_code': 200,
                    'id': librarian.id})


# 20. 删除图书管理员
@app.route('/func/librarian/delete', methods=['POST'])
@login_required
def delete_librarian():
    data = request.get_json()
    librarian = Librarian.query.filter_by(id=data['id']).first()
    if not librarian:
        return jsonify({'return_code': 301})
    db.session.delete(librarian)
    db.session.commit()
    return jsonify({'return_code': 200})

