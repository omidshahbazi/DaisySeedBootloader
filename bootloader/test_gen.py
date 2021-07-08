
with open("lorem.txt", 'r') as file:
  text = file.read()

text = [ord(c) for c in text]
text = bytes(text[:8192])

with open('test.bin', 'wb') as file:
  file.write(text)