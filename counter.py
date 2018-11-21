import sys

def printUsage():
  print("Usage:")
  print("python counter.py <file>")
  print("Counts number of all substrings longer than 3 in that file.")
  print("Prints percentage of substrgin occurrence frequencies.")

if len(sys.argv) < 2:
  print("Error! Need at least one argument.")
  printUsage()
  exit(1)

fileName = sys.argv[1]

numberOfSubstrings = 0.0
substringToOccurrenceNumber = {}

with open(fileName) as f:
  text = f.read()
  words = text.split()
  for word in words:
    if len(word) >= 4:
      for substrLength in range(4, 1 + len(word)):
	for offset in range(0, 1 + len(word) - substrLength):
	  subString = word[offset : offset + substrLength]
	  substringToOccurrenceNumber[subString] = substringToOccurrenceNumber.get(subString, 0) + 1
	  numberOfSubstrings += 1

results = []
for key, value in substringToOccurrenceNumber.items():
  results.append((value, key))
  
results = sorted(results, reverse=True)

print "Number of substrings onger than 3 is: ", numberOfSubstrings

id = 0
for r in results[:10]:
  print(id, 100 * r[0] / numberOfSubstrings, r[1])
  id += 1