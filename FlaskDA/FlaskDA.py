"""
 FlaskDA.py

  Modified  By  Reason
  --------  --  ------
  03-Feb-22 CBL Original, originates from web links below start of bringing
                all the pieces together for a DA system
                
  05-Feb-22 CBL Operational, adding in plotting. 

  09-Oct-22 CBL adding functionality. Still doing testing with the lassen GPS.

  06-Nov-22 CBL Menu packages aren't really that great. Adding more buttons
                and neating up the table of data.
                Need to set the limits by scale value.
                A lot of what is below replicates. Can I fix it to
                call a single template call. Also really need to work on
                the sizing of the label/numberical entry and set button.
                
  13-Nov-22 CBL condensed the way of handling callbacks/button pushes etc. 
                Added in bootstrap for menus and template inheritance

  17-Nov-22 CBL I don't think using an alarm funciton is the correct way to
                add points to the graph, changing to thread.

  20-Nov-22 CBL Moved most of this over to the __init__.py
  
  References:
  -----------
   https://medium.com/@rovai/from-data-to-graph-a-web-jorney-with-flask-and-sqlite-6c2ec9c0ad0
  https://wiki.gentoo.org/wiki/Gpib

 https://flask.palletsprojects.com/en/2.2.x/api/
 https://flask.palletsprojects.com/en/2.2.x/api/#url-route-registrations

 ===============================================================
 """
