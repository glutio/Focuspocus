# Focuspocus

If you wanted to create a GUI for your project, no matter how simple, there is some basic functionality that you would need to implement. For example on a touch screen you would need to be able to find what GUI element was tapped. Without a touch screen you would need to implement some sort of focus navigation. Even if your GUI does not take user input and is only for displaying information you will need to layout the UI elements and draw each element in the appropriate place on the screen. Focuspocus provides all this basic functionality out of the box. Use the provided framework to develop UI elements and let the library manage input and layout.

The library provides three main classes that form the foundation of the framework. `BView` is the base class for all UI elements (views). `BPanel` is a view that can contin other views. `BFocusManager` provides focus navigation, input event dispatching and the layout and render loop.

There are two layout panels, the base `BPanel` for manual positioning and `BStackPanel` for displaying elements in a row or a column. A view can have margins and a panel additionally has padding. The layout system allows for proportional sizing, fixed size, and autosize (size to content). A UI element's `width` or `height` property indicates how the layout panel should treat the element. If width (or height) is less than zero, then the width (height) of the UI element will be set by the panel during layout. If width (height) is greater than zero, then the width (height) value is used as is. If width (height) is zero, then the UI element is expected to calculate its own size based on its content. 

Input is implemented using virtual keyboard `BKeyboard` and virtual mouse `BMouse` classes. In your main loop read the input from your hardware devices, such as buttons or touchscreen, and convert it to virtual keyboard or virtual mouse events the framework recognizes. Override `handleEvent()` in your class derived from `BView` to respond to the events dispatched by the framework. User events can also be raised by the UI elements. Declare a custom event your view can trigger using `BEVENT()` macro.

Use the provided UI elements as examples of how to build a custom UI element for your project. The framework comes with a button, text label, checkbox and radiobutton and a scrollbar/slider. More coming soon.

## Layout

Every view has position and size properties that tell the rendering system where to draw the view. When views are placed onto a panel, the panel knows how to size and position the child views. `BPanel` is a fixed size fixed position panel, which does not provide any automatic layout. `BStackPanel` provides automatic sizing and layout based on available space, the view's margins and min/max size constraints.

### Size and position properties

A view's x,y position is relative to its parent panel and is the top left corner of the view's bounding box which includes margins. The `width` and `height` properties are used to indicate the layout mode. `BStackPanel` treats value less than 0 as proportional size, a value greater than zero as fixed size and a value of zero as size-to-content. During the measure pass the view gets a chance to autosize itself to its content by measuring all of it's children. After the layout `actualWidth` and `actualHeight` should be used to determine the view's size. In proportional size mode the `BStackPanel` respects the view's min/max constraints.

```
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;
  uint16_t minWidth;
  uint16_t minHeight;
  uint16_t maxWidth;
  uint16_t maxHeight;
  BMargin margin;
  int16_t actualWidth;
  int16_t actualHeight;
```

The layout system consists of two passes. The Measure pass and the Layout pass. During the measure pass the view gets a chance to autosize itself to its content by measuring all of it's children. Override `measure()` in your view if you want the view to set it's size based on its content. For example, by default `BTextLabel` has `width` and `height` set to zero, which means it should set `actualWidth` and `actualHeight` in the `measure()` implementation by measuring the bounds of the rendered text according to font and font size.

## Quickstart

Create a simple UI with two screens. Each screen is declared as a C++ namespace. The first screen is a "Click me" button. When clicked the button triggers onClick event which executes the handler function which pushes another screen on top of the screen stack. The new screen is a message box, made of a StackPanel a Label and a Button, clicking the button pops the screen off the screen stack revealing the previous screen.

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
