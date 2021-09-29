import sys
import os.path

if len(sys.argv) < 3 :
  print "ERROR: expected 2 arguments.\nUsage: stringify.py [--array] input output"
  sys.exit(1)

input_file_name = sys.argv[1]
output_file_name = sys.argv[2]
to_array = False

if input_file_name == "--array" :
  to_array = True
  input_file_name = sys.argv[2]
  output_file_name = sys.argv[3]

processed_lines = ["//\n// AUTO-GENERATED FILE - DO NOT EDIT!!\n//\n\n"]


if to_array :
  f = open(input_file_name, 'rb')
  bytes = f.read()
  f.close()
  cnt = 0
  for b in bytes:
    val = ord(b);
    processed_lines.append(str(val) + ",")
    cnt = cnt + 1
    if cnt > 30 or val == 10 :
      cnt = 0
      processed_lines.append("\n")
  processed_lines.append("0\n")

else :
  f = open(input_file_name, 'rt')
  for line in f:
    line = line.rstrip("\n\r")
    substrings = len(line) / 80 + 1
    for i in xrange(substrings):
      s = "\"" + line[i * 80 : (i + 1) * 80].replace("\\", "\\\\").replace("\"", "\\\"")
      if i == substrings - 1:
        s = s + "\\n\"\n"
      else:
        s = s + "\"\n"
      processed_lines.append(s)
  f.close()


is_file_changed = False

if os.path.isfile(output_file_name):
  existing_lines = []
  f = open(output_file_name, 'rt')
  for line in f:
    existing_lines.append(line);
  f.close()

  if len(processed_lines) != len(existing_lines):
    is_file_changed = True
  else:
    for i in xrange(len(processed_lines)):
      if processed_lines[i] != existing_lines[i]:
        is_file_changed = True
        break

else:
  is_file_changed = True

if is_file_changed:
  f = open(output_file_name, 'wt')
  for line in processed_lines:
    f.write(line)
  f.close()
  print "Stringified output: ", output_file_name
