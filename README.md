# ESP32 Generative Art

This generative art display shows a scrolling text message with an ASCII icon in the background, moving around the screen at random. The icon bounces off of the edges of the display, and when it reaches a corner, the scrolling text changes to the next in a set of random messages. The messages include quotes from early computer user guides, emphasizing the strangeness of a world without intrinsic computer knowledge as well as how surprisingly similar computer interaction was in its early days as it is today. Click the video below to see a random sequence of corner hits that shows all of the messages and icons on the display.

[![Full animation video](https://img.youtube.com/vi/LdZ8YPkkrzI/0.jpg)](https://www.youtube.com/watch?v=LdZ8YPkkrzI)

This visual was initially a part of a group of ESP32 displays that were shown together in a hanging formation. Click the image below to see a video of the hanging and spinning displays.

[![all class video](https://img.youtube.com/vi/9MEQ5M5bH7o/0.jpg)](https://www.youtube.com/watch?v=9MEQ5M5bH7o)

The generative display is programmed in C++ on an ESP32 microcontroller. This program is run using [PlatformIO](https://platformio.org/), which allows C++ code to be uploaded via USB serial connection onto an ESP32. It uses the Arduino [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) library for graphics and fonts, which can be installed via the Arduino IDE.

The program uses a sprite to display the scrolling text and a second transparent sprite to display the icon. The icon moves with a random velocity until it reaches an edge, then bounces off in a semi-random direction that is bounded so the icon doesn’t get into a horizontal or vertical bounce pattern that never reaches a corner.

The definition of a "corner" also has a slight buffer of 1/20th of the screen, which increases the corner hit rate and allows the message to change more frequently. This is a small enough margin so that corner hits still appear visually precise in most cases, but doesn’t require the icon to hit a singular perfect point to trigger the message change.

Every one minute, an intro sequence is played that provides context for the viewer and also an upper bound on the amount of time a single message can play because the intro sequence automatically triggers a random switch to another message.
