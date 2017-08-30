# Voronoi

Windows: [![Build status](https://ci.appveyor.com/api/projects/status/l0yuxruih8f6rutc?svg=true)](https://ci.appveyor.com/project/JonnyPtn/voronoi)

Linux: [ ![Codeship Status for JonnyPtn/Voronoi](https://app.codeship.com/projects/993e2000-bef9-0134-6367-0ef15c5d34cb/status?branch=sfvoronoi)](https://app.codeship.com/projects/196443)

by Mark Dally

https://github.com/mdally/Voronoi

Modified for use with SFML by Jonny Paton

http://www.sfml-dev.org/

================================================

A simple library for computing Voronoi diagrams using Fortune's algorithm and performing Lloyd's relaxation.

Cmake included, lib and example (in examples folder) should support all platforms supported by SFML

###Usage:
//compute the diagram for a set of sites and a bounding box
```
VoronoiDiagramGenerator::compute(std::vector<Point2>& sites, BoundingBox bbox)
```

//perform Lloyd's relaxation on the diagram last computed
```
VoronoiDiagramGenerator::relax()
```

###Notes:
 * It is your responsibility to ensure that there are no duplicate sites or sites that fall outside or on the borders of the bounding box.
	
 * Performing Lloyd's relaxation returns a new diagram but does not delete the original. You must delete the old one in order to avoid memory leaks.
