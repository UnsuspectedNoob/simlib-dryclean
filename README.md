# Dry Cleaning Simulation – SIMLIB-Based

This repository contains the complete implementation of a dry cleaning simulation program using the **SIMLIB** event-driven simulation library.

## 📁 Files Included

| File              | Description                                      |
|-------------------|--------------------------------------------------|
| `dryclean.c`      | Main simulation program                          |
| `simlib.c/h`      | SIMLIB library source and header files           |
| `lcgrand.c/h`     | Random number generator implementation           |
| `simlibdefs.h`    | Simulation parameter definitions                 |
| `input.txt`       | Input file required by the executable            |
| `dryclean.exe`    | Windows executable compiled from the code        |

> 💡 All files are self-contained — no external libraries required.

---

## 🚀 How to Run the Simulation

1. **Download the repository** as a `.zip` file:
   - Click the green **Code** button (top right of this page)
   - Select **"Download ZIP"**
   - Extract the ZIP archive on your Windows system

2. **Prepare input**:
   - Ensure the extracted folder contains the `input.txt` and `dryclean.exe` files in the **same directory**
   - The `input.txt` must contain **two space-separated values** on a single line:
     ```
     <mean_interarrival_time> <total_simulation_time>
     ```
     Example:
     ```
     3.5 10000
     ```

3. **Run the program**:
   - Double-click `dryclean.exe`
   - The program will read from `input.txt` and output results to the console or terminal

---

## ⚠️ Notes

- If your browser warns about downloading `dryclean.exe`, you can proceed by allowing the download — the file is safe and generated directly from the source code in this repository.
- If you prefer an alternate format or delivery method, please contact me.

---

## 📫 Contact

**Author**: UKPAI, Munachiso 
**Email**: ukpaimunachi2003@gmail.com 
**GitHub**: [UnsuspectedNoob](https://github.com/UnsuspectedNoob)

