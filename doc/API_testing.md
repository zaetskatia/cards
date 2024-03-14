# Server Application API Testing Documentation
## Overview
This document provides guidelines for testing the Server Application API using Postman. The API supports operations to create, retrieve, update, and delete (CRUD) cards and folders. Additionally, user authentication is required for accessing these resources.

Base URL
The base URL for the API is:
```
 http://localhost:8080/
```

Replace yourserveraddress and port with your actual server address and port number.

### User Authentication
 **Sign Up**
   - `POST /signup`
   - Body:
     ```
     {
       "username": "newUser",
       "password": "newPassword"
     }
     ```

**Sign In**
   - `POST /signin`
   - Body:
     ```
     {
       "username": "existingUser",
       "password": "existingPassword"
     }
     ```

**Log Out**
   - `POST /logout`
   - Header: `Authorization: Bearer <token>`

### Folders
**Create a Folder**
   - `POST /folders`
   - Header: `Authorization: Bearer <token>`
   - Body:
     ```
     {
       "name": "My New Folder"
     }
     ```

**Get All Folders**
   - `GET /folders`
   - Header: `Authorization: Bearer <token>`

**Get One Folder**
   - `GET /folders/{folderId}`
   - Header: `Authorization: Bearer <token>`

**Update a Folder**
   - `PUT /folders/{folderId}`
   - Header: `Authorization: Bearer <token>`
   - Body:
     ```
     {
       "name": "Updated Folder Name"
     }
     ```

**Delete a Folder**
   - `DELETE /folders/{folderId}`
   - Header: `Authorization: Bearer <token>`

### Cards
**Create a Card**
   - `POST /cards`
   - Header: `Authorization: Bearer <token>`
   - Body:
     ```
     {
       "folderId": 1,
       "term": "Term",
       "translation": "Translation"
     }
     ```

**Get All Cards in a Folder**
   - `GET /cards?folderId=1`
   - Header: `Authorization: Bearer <token>`

**Get One Card**
   - `GET /cards/{cardId}`
   - Header: `Authorization: Bearer <token>`

**Update a Card**
   - `PUT /cards/{cardId}`
   - Header: `Authorization: Bearer <token>`
   - Body:
     ```
     {
       "folderId": 1,
       "term": "Updated Term",
       "translation": "Updated Translation"
     }
     ```

**Delete a Card**
   - `DELETE /cards/{cardId}`
   - Header: `Authorization: Bearer <token>`
