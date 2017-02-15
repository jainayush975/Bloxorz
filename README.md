#####BLOXORZ
===========================
	In this game, there is a movable cuboidal block. The goal of the player is to make it fall into a square hole
	This game build in cpp using openGL library for graphics.
    	This is a 2D game.


#####Game description :
-------------------------
  * In game we need to go to hole.
	* The Block can rotate along any of its axis.
	* You have particular number of JUMPS.
	* You need to minimise total moves.
  * If you fall out side, Game Over.
  * Moves are shown on Right Top corner of screen.
	* Time taken is shown on left Top corner of screen.
	* AVAILABLE JUMPS are shown on Center Top of screen.
	* You can change the view to :
				(i)  TOP view
				(ii) Block view
				(iii) Tower view
				(iv) Follow-cam view
				(v)  Helicopter-cam view
	* We have 5 types of tile :
				(i)  Normal Tile :- BLUE COLOUR
				(ii) Fragile Tile :- RED or orange
				(iii) BRIDGE TILE :- BLUE (Opened by switch)
				(iv) Switch TILE :- GREEN
				(v)  Block and Destination Tile :- YELLOW


#####Game Controls:

    Keyboard:
      * up , down - To move the block up or down on plane respectively.
      * right,left - To move the block right and left on plane respectively.
      * w , s - To JUMP the block up or down on plane respectively.
      * a , d - To JUMP the block right and left on plane respectively.
      * c  - To change camera.
			* v - Change side of view in follow or self mode of camera.
      * mouse scroll -  To zoom in and zoom out the game window respectively.
      * Spacebar - To see Helicopter-cam.

    Mouse:
			* Scroll up to zoom in and down to zoom out.


####Game End:
      * If block fall, Game Over.

####Cheers enjoy the game!!


####Additional Features
  * Score display using 7 segment.
  * Added Background music to make game more interesting.
	* Added special sound effects for different actoins.
	* Different views
  * Mouse + keyboard controls.
	

### Dependencies:
##### Linux/Windows/ Mac OSX - Dependencies: (Recommended)
* GLFW
* GLAD
* GLM

##### Linux - Dependencies: (alternative)
* FreeGLUT
* GLEW
* GLM
