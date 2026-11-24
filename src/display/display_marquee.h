#ifndef DISPLAY_MARQUEE_H
#define DISPLAY_MARQUEE_H

#define MARQUEE_SPEED 20
#define MARQUEE_MAX_HEIGHT 16
#define NOTIFICATION_MAX_CHARS 120

void drawMarqueeBorders();
void drawMarquee();
void setNotificationText(const char* text);
void updateMarquee();

#endif