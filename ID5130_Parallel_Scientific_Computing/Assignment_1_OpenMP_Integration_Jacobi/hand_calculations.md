# ID5130 Assignment 1 — Analytical Hand Calculations

## 1. Analytical Evaluation of the Triple Integral

We need to evaluate:

    I = ∫₀⁴ ∫₀³ ∫₀² (4x³ + xy² + 5y + yz + 6z) dz dy dx

### Step 1: Inner integral over z (limits 0 to 2)

    ∫₀² (4x³ + xy² + 5y + yz + 6z) dz

    = [4x³z + xy²z + 5yz + yz²/2 + 3z²]₀²

    = 4x³(2) + xy²(2) + 5y(2) + y(4)/2 + 3(4)

    = 8x³ + 2xy² + 10y + 2y + 12

    = 8x³ + 2xy² + 12y + 12

### Step 2: Middle integral over y (limits 0 to 3)

    ∫₀³ (8x³ + 2xy² + 12y + 12) dy

    = [8x³y + 2xy³/3 + 6y² + 12y]₀³

    = 8x³(3) + 2x(27)/3 + 6(9) + 12(3)

    = 24x³ + 18x + 54 + 36

    = 24x³ + 18x + 90

### Step 3: Outer integral over x (limits 0 to 4)

    ∫₀⁴ (24x³ + 18x + 90) dx

    = [6x⁴ + 9x² + 90x]₀⁴

    = 6(256) + 9(16) + 90(4)

    = 1536 + 144 + 360

    = **2040**


## 2. Exact Solution for Poisson Equation (Verification)

Given:  ∇²φ = -q,  where q = 2(2 - x² - y²)

Claimed exact solution: φ(x,y) = (x² - 1)(y² - 1)

### Verification:

    φ = (x² - 1)(y² - 1) = x²y² - x² - y² + 1

    ∂φ/∂x = 2xy² - 2x
    ∂²φ/∂x² = 2y² - 2

    ∂φ/∂y = 2x²y - 2y
    ∂²φ/∂y² = 2x² - 2

    ∇²φ = ∂²φ/∂x² + ∂²φ/∂y²
         = (2y² - 2) + (2x² - 2)
         = 2x² + 2y² - 4
         = -(4 - 2x² - 2y²)
         = -2(2 - x² - y²)
         = -q  ✓

### Boundary conditions check:

    φ(±1, y) = (1 - 1)(y² - 1) = 0  ✓
    φ(x, ±1) = (x² - 1)(1 - 1) = 0  ✓

### Maximum value of φ:

    φ(0, 0) = (0 - 1)(0 - 1) = 1  (maximum at center of domain)

At y = 0.5: φ(x, 0.5) = (x² - 1)(0.25 - 1) = -0.75(x² - 1) = 0.75(1 - x²)
    φ(0, 0.5) = 0.75  (peak of the profile plotted in Q2a)
