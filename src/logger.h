#pragma once

#include <daScript/daScript.h>
#include <vector>
#include <string>
#include <sstream>

#define NORMAL_LINE_COLOR     0xFFFFFFFF
#define ERROR_LINE_COLOR      0xFFFFD080
#define WARNING_LINE_COLOR    0xFFB0B0B0
#define NOTE_LINE_COLOR       0xFFA0A0FF
#define SCROLL_POSITION_COLOR 0xFF909090

enum
{
  LOGGER_NORMAL,
  LOGGER_ERROR,
  LOGGER_WARNING,
  LOGGER_NOTE,
};


class DasboxLogger : public das::TextWriter
{
public:
  std::stringstream cerrStream;
  std::stringstream coutStream;
  std::vector<std::string> logStrings;
  std::vector<uint32_t> lineColors;
  int topErrorLine = -1;
  int topLine = 0;

  virtual void output() override;

  void setTopErrorLine();
  void clear();
  void printAllLog();

  void setState(int state_)
  {
    state = state_;
    applyStateColor();
  }

protected:
  int pos = 0;
  int state = LOGGER_NORMAL;
  uint32_t curColor = NORMAL_LINE_COLOR;
  std::string buf;

  uint32_t setLogColor(uint32_t color);
  void addString(const std::string & s);

  void applyStateColor();
};

extern DasboxLogger dasbox_logger;

void on_switch_to_log_screen();
void on_return_from_log_screen();
void update_log_screen(float dt);
void draw_log_screen();
