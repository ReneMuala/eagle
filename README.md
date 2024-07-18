![logo](images/eagle.jpeg)

# The Eagle Project

Eagle is an OCR engine written in C++ and using Tesseract, and OpenCV.

## Getting Started
### Building
```bash
git clone https://github.com/renemuala/eagle # clone a fresh copy of the repository
xmake # build the project
```

### Running
```bash
xmake run eagle fake/path/picture.jpeg # run eagle in filesystem mode
# or
xmake run eagle http 1234  # run eagle in http mode at port 1234

```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
