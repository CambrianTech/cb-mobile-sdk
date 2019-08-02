# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in /Users/joseph/Library/Android/sdk/tools/proguard/proguard-android.txt
# You can edit the include path and order by changing the proguardFiles
# directive in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}


-optimizationpasses 2
-dontwarn sun.misc.Unsafe
-dontwarn java.lang.invoke.*
-dontwarn com.koushikdutta.**

-keep class android.support.v7.widget.SearchView { *; }
-keepattributes *Annotation*
-keepattributes SourceFile,LineNumberTable

# AWS exclusions
# Class names are needed in reflection
-keepnames class com.amazonaws.**
-keepnames class com.amazon.**
# Request handlers defined in request.handlers
-keep class com.amazonaws.services.**.*Handler
# The following are referenced but aren't required to run
-dontwarn com.fasterxml.jackson.**
-dontwarn org.apache.commons.logging.**
# Android 6.0 release removes support for the Apache HTTP client
-dontwarn org.apache.http.**
# The SDK has several references of Apache HTTP client
-dontwarn com.amazonaws.http.**
-dontwarn com.amazonaws.metrics.**


-keepclasseswithmembernames class * {
    native <methods>;
}

-keep class Loader {
    public static void main(...);
}

-keep class com.cambrian.cbar.remodeling.CBColorFinderView {
    public void colorsFound(int[], float[], float[]);
}

-keep class com.cambrian.cbar.ICBJavaObject {
    long getCoreObject();
}

-keep class com.cambrian.cbar.remodeling.CBRemodeling* {
    <init>(java.lang.String);
}

-keep class com.cambrian.cbar.CBAugmentedScene { void appendAsset(com.cambrian.cbar.CBAugmentedAsset); }