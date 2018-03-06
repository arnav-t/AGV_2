# AGV_2
## Lane detection task for AGV
### Working   
1. Segment the image into horizontal strips (in this case, 10)
2. Determine the lower threshold for Canny edge detection by "adaptive" thresholding (mean).
3. Apply Canny edge detector to individual strips.
4. Apply Probabilistic Hough Line Transform to get all the lines in the strip with different thresholds according to the strip positon such that it is slightly biased towards finding lines in the bottom part of the screen.
5. Extend all the lines in a section such that they are clipped by the strip and reject the lines if they are too horizontal.
6. Take all possible unordered pairs of lines and vote for the horizon while storing all the lines that voted for that particular horizon.
7. Find the horizon with highest amount of votes and draw the corresponding lines to it. (and its neighbouring horizon within the allowed thickness of horizon)
