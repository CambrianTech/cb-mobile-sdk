
First. Open Disk Utility and create a new Image->Blank and choose a case sensitive file system type along with enough space "10 GB"

To find empty directories
``` 
find ./* -type d -empty 
```

To list which files:
```
find ./* -depth -type d -exec rename -n 's!/([^/]*/?)$!\L/$1!' {} +
```

To execute:
```
find ./* -depth -type d -exec rename 's!/([^/]*/?)$!\L/$1!' {} +
```

To resize images:
https://askubuntu.com/questions/135477/how-can-i-scale-all-images-in-a-folder-to-the-same-width

```
brew install imagemagick
```


Resize all images recursively:
```
find . -name '*.jpg' -exec mogrify -resize 1024x1024\> -format jpg {} + 
```

Rename sara's files:

#Base_Color.jpg:
find ./* -type f -name '*Base_Color.jpg' -exec rename 's/\.\/(.+)\/.+Base_Color.jpg$/$1\/Base_Color.jpg/' '{}' \;

#Normal.jpg:
find ./* -type f -name '*Normal.jpg' -exec rename 's/\.\/(.+)\/.+Normal.jpg$/$1\/Normal.jpg/' '{}' \;

#Roughness.jpg:
find ./* -type f -name '*Roughness.jpg' -exec rename 's/\.\/(.+)\/.+Roughness.jpg$/$1\/Roughness.jpg/' '{}' \;
