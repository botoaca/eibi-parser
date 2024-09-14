# Eibi Parser
eibi-parser is a utility made to parse and search the eibi schedule into more human-readable sections.

# Usage
all these arguments can be composed to form a more specific query.
| Command-line argument | Purpose |
|-----------------------|---------|
| --name | Search only for stations with that name |
| -f / --freq | Search only for stations on that frequency |
| -t / --time | Search only for stations active at that time in the UTC timezone (i.e. 0100 being 1 AM) |
| --country | Search only for stations broadcasting from that country |
| -lang / --language | Search only for stations broadcasting in that language |
| --target | Search only for stations targetting that area |