# Focuspocus

GUI core for Arduino. Create your own UI elements. The library provides the GUI framework which you can use to create the UI you want. Easily create custom UI elements and let the framework take care of layout and rendering, focus navigation and dispatching input events. 

There are three main classes `BView`, `BPanel` and `BFocusManager`. All UI elements derive from `BView` including `BPanel` which is a UI element that can contain UI elements. `BFocusManager` is provides the GUI render and event loop, focus navigation and holds the referrence to the screen that's currently active.

## Quickstart

Create a simple UI. Each screen is declared as a C++ namespace. The first screen is a "Click me" button. When clicked the button triggers onClick event which executes the handler function which pushes another screen on top of the screen stack. The new screen is a message box, made of a StackPanel a Label and a Button. 

![Focuspocus - Wokwi ESP32, STM32, Arduino Simulator and 11 more pages - Work - Microsoft_ Edge 2023-06-27 06-50-25 (online-video-cutter com)](https://github.com/glutio/Focuspocus/assets/22550674/25cf0087-8195-406b-9b31-f1989eaa7ac1)

```
namespace HelloWorld {
  BTextLabel label; // declare a label
  BButton button; // declare a button
  BView* mainContent[] { &label, &button }; // create content array
  BStackPanel main(mainContent); // create panel with content
  BRootView root(main); // create the root view from the main panel

  // button click handler
  void handleButtonClick(BButton* sender, bool isClicked) {
    if (isClicked) { // if click not cancelled
      // pop the top screen from the stack
      sender->focusManager().pop();
    }
  }

  // initialize screen content
  void init() {
    label.text = "Hello world!";

    button.text = "Ok";
    button.onClick += handleButtonClick;

    main.spacing = 10;
    main.orientation = B::vertical;
    main.horizontalAlignment = B::center;
    main.border = true;
    main.padding(5);

    root.focused = &button;
  }
}

// create a namespace for screen content
namespace MainScreen {
  BButton button; // declare a button
  BView* mainContent[] = { &button }; // create the content array
  BStackPanel main(mainContent); // create a panel with content
  BRootView root(main); // create the root view from the main panel

  // declare an event handler
  void handleButtonClick(BButton* sender, bool isClicked) {
    if (isClicked) { // if the click was not cancelled
      // push a screen onto the stack
      sender->focusManager().push(HelloWorld::root);
    }
  }

  // init screen content
  void init() {
    button.text = "Click me";
    button.fontSize = 2;
    button.onClick += handleButtonClick;

    main.verticalAlignment = B::center;
    main.horizontalAlignment = B::center;
    main.width = -1; // full screen
    main.height = -1; // fullscreen

    root.focused = &button;
  }  
}
```
