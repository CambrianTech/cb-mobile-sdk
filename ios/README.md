# cb-ios


### common problems
If XCode opens up and dies when you attempt to load the project, it's probably due to derived data. To fix the problem, delete it:
```
rm -rf ~/Library/Developer/Xcode/DerivedData
```

You'll have to do this a lot, it would be a good idea to make an alias; add the following to your .bash_profile:
```
alias ddd="rm -rf ~/Library/Developer/Xcode/DerivedData"
```

Install command line tools for xcode
```
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
```

Install carthage support:
```
brew install carthage
```

To enable carthage on an app, navigate to its directory and run:

```
cb carthage-sync clean
cb carthage-sync
```
