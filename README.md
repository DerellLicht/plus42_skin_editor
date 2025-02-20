An editor to modify/resize Free42/Plus42 skins 

This editor for Plus42 calculator skins, is primarily designed for resizing 
existing skins.  
Steps for using the program are:

1. copy an existing Plus42 skin, both the name.layout and name.gif files,
   to a new filename.
   
2. resize the new_name.gif file to the desired new size.
   It is recommended to make width field an even number, to avoid ambiguous results.

3. Update Size field in the .layout file:  

3a. Duplicate the Size field, and comment out the original Size field  

3b. update new Size: field to reflect the new image size.  
   NOTE that the Size: Width field is half of the image size, since the GIF is two images wide.
   
Example:
```
#Skin: 0,0,1280,656
Skin: 0,0,950,487
```   
   
4. run the plus42_skin_editor program; syntax is:

   Usage: plus42_skin_editor <target_layout_file_name>
   
5. copy the updated .layout and .gif files to your Plus42 program folder,
   then select the new skin from within the program.
      