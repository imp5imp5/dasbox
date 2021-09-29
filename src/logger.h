#pragma once

#include <daScript/daScript.h>
#include <vector>
#include <string>
#include <sstream>

#define NORMAL_LINE_COLOR     0xFFFFFFFF
#define ERROR_LINE_COLOR      0xFF80D0FF
#define NOTE_LINE_COLOR       0xFFFFA0A0
#define SCROLL_POSITION_COLOR 0xFF909090

class Logger : public das::TextWriter
{
public:
  std::stringstream cerrStream;
  std::stringstream coutStream;
  std::vector<std::string> logStrings;
  std::vector<uint32_t> lineColors;
  int topErrorLine = -1;
  int topLine = 0;

  virtual void output() override;
  uint32_t setLogColor(uint32_t color);

  void setTopErrorLine();
  void clear();

protected:
  int pos = 0;
  uint32_t curColor = NORMAL_LINE_COLOR;
  std::string buf;

  void addString(const std::string & s);
};

extern Logger logger;

void on_switch_to_log_screen();
void update_log_screen(float dt);
void draw_log_screen();
