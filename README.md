# miniDBMS
![레이어드아키텍처](https://user-images.githubusercontent.com/61873510/127859797-80d580ee-089e-45ca-ac46-6da16221a4b9.png)
This is the mini DataBase Management System consists of file manager layer, buffer manager layer, B+ Tree layer, api layer and lock table layer.<br><br>
**Api lists** : ```init_db()```, ```open_table()```, ```db_insert()```, ```trx_begin()```, ```trx_commit()```, ```db_find()```, ```db_update()```,  ```close_table()```, ```shutdown_db()``` <br><br>
It can store 128byte sized key-value strings to the disk using B+ Tree algorithm.<br> <br>
```db_delete()``` operation is not available.<br><br>

See the project report(Korean) at the ```final_report``` folder to see the full report.<br><br>
