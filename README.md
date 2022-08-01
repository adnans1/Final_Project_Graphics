Graphic Engine based on Libigl

For compiling:

1.	Clone or download the project
2.	Download Cmake from the link in the site
3.	Run Cmake gui. choose the project folder and destination folder for the cpp project files. choose after pressing configure choose compiler (VS2019 for example). After finish configuration successfully, press configure again and after it finishes press generate.
4.	If everything pass successfully got to the destination folder and launch the project.
5.	Copy configuration.txt from tutorial/sandBox to build/tutorial/sandBox sandBox as a startup project and compile the project (it could take few mineutes);


	object move

1.	each object has a unique bezier curve to follow (as shown in class) while animate
2.	the object's bezier curve can be modified by using the bezier curve editor on the right side of the screen.
3.	each object has a delay factor to wait until starting animation can be changed by the text box named “Time”. 
for example: if you write “3” in the “Time” text box, and clicked “set time”. 
Now when you click on “Play” the animation will start according to the draw in the right after 3 seconds.

	object layers:

1.	each object is assigned to a layer
2.	new objects will be created, after adding the object you can choose it’s unique layer.
3.	layers can be hidden to hide all shapes assigned to the layer by using the checkbox with the layer's number.
	the metirial menu:
1.	the user can load .png file to make a new metirial to the presented objects, by clicking on the "Add" button on the Metirials menu.

	Background :

1.	the background is a cube map of the sky by default.
2.	the user can change the Background by selecting one of the option list in the menu.

	Transperent :

1.	each object can be chosen to be transparent. 
2.	the transparent object has a dropdown list to choose the transparent percentage of the accuracy.

	 Animation :

1.	move the shape: after selecting a single shape the user can edit the shape function by moving the green dots.
2.	while clicking on space key , or on play on the menu, the animation starts.
3.	You can determine the speed of the object move by adding a number to the “speed” textbox and click on set speed 
