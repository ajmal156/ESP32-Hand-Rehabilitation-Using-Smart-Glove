from flask import Flask, request
import csv
import os
from datetime import datetime


app = Flask(__name__)

# Files and folders


USER_FILE = "patients.csv"

PATIENT_FOLDER = "patients"


current_patient = None


# Create patients folder

if not os.path.exists(PATIENT_FOLDER):

    os.makedirs(PATIENT_FOLDER)

# Create patients.csv automatically

if not os.path.exists(USER_FILE):

    with open(USER_FILE, "w", newline="") as file:

        writer = csv.writer(file)

        writer.writerow(
            [
                "username",
                "password",
                "name"
            ]
        )



    print("patients.csv created")

# Create patient CSV file

def create_patient_file(name):


    filename = (PATIENT_FOLDER + "/"+ name + ".csv" )
    
    if not os.path.exists(filename):


        with open(filename,"w",newline="") as file:


            writer = csv.writer(file)


            writer.writerow(["Date","Time","Exercise","Duration","Status"])
            print("Patient file created:",filename)


# Login System

@app.route("/login", methods=["POST"])

def login():

    global current_patient


    data = request.json

    username = data["username"]

    password = data["password"]

    users = []


    # Read existing users

    with open(USER_FILE,"r") as file:

        reader = csv.DictReader(file)

        for user in reader:

            users.append(user)

  # Check existing user

    for user in users:


        if user["username"] == username:



            if user["password"] == password:


                current_patient = user["name"]


                create_patient_file(current_patient)

                print("Existing user login:", current_patient)
                
                return "SUCCESS"
            else:
                return "WRONG PASSWORD"


    # Create New User
  
    with open(USER_FILE,"a",newline="") as file:
        
        writer = csv.writer(file)
        
        writer.writerow([username , password , username])
        
    current_patient = username

    create_patient_file(username)

    print("New Patient Created:",username)
    
    return "SUCCESS"


# Receive ESP32 Data


@app.route("/save", methods=["POST"])

def save_data():

    global current_patient

    if current_patient is None:

        return "NO USER LOGIN"


    data = request.json

    exercise = data["exercise"]

    duration = data["duration"]

    status = data["status"]


    filename = (PATIENT_FOLDER+ "/" + current_patient + ".csv")

    now = datetime.now()

    with open(filename, "a", newline="") as file:
        writer = csv.writer(file)
        
        writer.writerow([now.date(), now.strftime ("%H:%M:%S"), exercise, duration, status])



    print("Therapy Data Saved:",exercise)


    return "SAVED"

# Start Flask Server

if __name__ == "__main__":


    app.run(host="0.0.0.0", port = 5000)