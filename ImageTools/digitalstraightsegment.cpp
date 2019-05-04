#include <iostream>
#include "digitalstraightsegment.h"



DigitalStraightSegment::DigitalStraightSegment (Pt2i p1, Pt2i p2, int type,
                                 int xmin, int ymin, int xmax, int ymax)
                      : DigitalStraightLine (p1, p2, type)
{
  if (a < (b < 0 ? -b : b))
  {
    min = xmin;
    max = xmax;
  }
  else
  {
    min = ymin;
    max = ymax;
  }
}


DigitalStraightSegment::DigitalStraightSegment (Pt2i p1, Pt2i p2, Pt2i p3,
                                 int xmin, int ymin, int xmax, int ymax)
                      : DigitalStraightLine (p1, p2, p3)
{
  if (a < (b < 0 ? -b : b))
  {
    min = xmin;
    max = xmax;
  }
  else
  {
    min = ymin;
    max = ymax;
  }
}


DigitalStraightSegment::DigitalStraightSegment (Pt2i p1, Pt2i p2, int width)
                      : DigitalStraightLine (p1, p2, DSL_THIN)
{
  nu = width * period ();
  c = a * p1.x () + b * p1.y () - nu / 2;
  if (a < (b < 0 ? -b : b))
  {
    min = (p1.x () < p2.x () ? p1.x () : p2.x ());
    max = (p1.x () < p2.x () ? p2.x () : p1.x ());
  }
  else
  {
    min = (p1.y () < p2.y () ? p1.y () : p2.y ());
    max = (p1.y () < p2.y () ? p2.y () : p1.y ());
  }
}


DigitalStraightSegment::DigitalStraightSegment (int va, int vb, int vc,
                                                int vnu, int vmin, int vmax)
                      : DigitalStraightLine (va, vb, vc, vnu)
{
  min = vmin;
  max = vmax;
}


Pt2i DigitalStraightSegment::getABoundingPoint (bool upper) const
{
  int sa = a, sb = b, u1 = 1, v1 = 0, u2 = 0, v2 = 1;
  while (sb != 0)
  {
    int r = sa % sb;
    int q = sa / sb;
    int u3 = u1 - q * u2;
    int v3 = v1 - q * v2;
    u1 = u2;
    v1 = v2;
    u2 = u3;
    v2 = v3;
    sa = sb;
    sb = r;
  }
  if (sa < 0)  // should be 1 or -1 if a and b are primal
  {
    u1 = - u1;   // necessary if sa = -1
    v1 = - v1;
  }
  Pt2i extr (upper ? Pt2i (u1 * (c + nu - 1), v1 * (c + nu - 1))
                   : Pt2i (u1 * c, v1 * c));
  
  int dec = 0;
  int bb = (b < 0 ? -b : b);
  if (a < bb)
  {
    if (extr.x () > max)
      dec = 1 + (extr.x () - max) / bb;
    else if (extr.x () < min)
      dec = -1 - (min - extr.x ()) / bb;
    if (b < 0) dec = -dec;
  }
  else
  {
    if (extr.y () > max)
      dec = -1 - (extr.y () - max) / a;
    else if (extr.y () < min)
      dec = 1 + (min - extr.y ()) / a;
  }
  extr.set (extr.x () - dec * b, extr.y () + dec * a);
  return extr;
}


void DigitalStraightSegment::adjustWorkArea (int &xmin, int &ymin,
                                             int &width, int &height) const
{
  if (b > a || -b > a)
  {
    if (xmin < min) xmin = min;
    // Caution, segment max limit is excluded
    int x2 = (xmin + width < max + 1 ? xmin + width : max + 1);
    width = (xmin >= x2 ? 0 : x2 - xmin);
  }
  else
  {
    if (ymin < min) ymin = min;
    // Cochon, segment max limit is excluded
    int y2 = (ymin + height < max + 1 ? ymin + height : max + 1);
    height = (ymin >= y2 ? 0 : y2 - ymin);
  }
}


void DigitalStraightSegment::getPoints (vector<Pt2i> &pts) const
{
  int xmin, ymin, w, h;
  if (b > a || -b > a)
  {
    xmin = min;
    w = max - min;
    ymin = (b < 0 ? (c + nu - a * min) / b - 1 : (c - a * max) / b - 1);
    h = (b < 0 ? (c - a * max) / b + 1 : (c + nu - a * min) / b + 1) - ymin;
  }
  else
  {
    ymin = min;
    h = max - min;
    xmin = (b < 0 ? (c - b * min) / a - 1 : (c - b * max) / a - 1);
    w = (b < 0 ? (c + nu - b * max) / a : (c + nu - b * min) / a) + 1 - xmin;
  }
  vector<Pt2i> lowbound;
  getBoundPoints (lowbound, false, xmin, ymin, w, h);
  vector<Pt2i>::iterator it = lowbound.begin ();

  while (it != lowbound.end ())
  {
    Pt2i p = *it;
    while (owns (p))
    {
      pts.push_back (p);
      if (b > a) p.set (p.x (), p.y () + 1);
      else if (-b > a) p.set (p.x (), p.y () - 1);
      else p.set (p.x () + 1, p.y ());
    }
    it ++;
  }
}


void DigitalStraightSegment::naiveLine (AbsRat &x1, AbsRat &y1,
                                        AbsRat &x2, AbsRat &y2) const
{
  if (a < (b < 0 ? -b : b))
  {
    x1.set (min, 1);
    y1.set (2 * c + nu - 1 - 2 * a * min, 2 * b);
    x2.set (max, 1);
    y2.set (2 * c + nu - 1 - 2 * a * max, 2 * b);
  }
  else
  {
    y1.set (min, 1);
    x1.set (2 * c + nu - 1 - 2 * b * min, 2 * a);
    y2.set (max, 1);
    x2.set (2 * c + nu - 1 - 2 * b * max, 2 * a);
  }
}


DigitalStraightSegment *DigitalStraightSegment::erosion (int num, int den) const
{
  int newwidth = nu;
  if (nu > period ())
  {
    newwidth = nu - (num * period ()) / den;
    if (newwidth < period ()) newwidth = period ();
  }
  return (new DigitalStraightSegment (a, b, c + (nu - newwidth) / 2,
                                      newwidth, min, max));
}


DigitalStraightSegment *DigitalStraightSegment::dilation (
                                                   int num, int den) const
{
  int newwidth = nu + (num * period ()) / den;
  return (new DigitalStraightSegment (a, b, c + (nu - newwidth) / 2,
                                      newwidth, min, max));
}


DigitalStraightSegment *DigitalStraightSegment::dilation (int radius) const
{
  return (new DigitalStraightSegment (a, b, c - radius,
                                      nu + 2 * radius, min, max));
}


void DigitalStraightSegment::dilate (int radius)
{
  nu += 2 * radius;
  c -= radius;
}


bool DigitalStraightSegment::contains (Pt2i p, int tol) const
{
  int pos = a * p.x () + b * p.y ();
  tol *= period ();
  if (pos < c - tol || pos >= c + nu + tol) return (false);
  if (a < (b < 0 ? -b : b))
    return (p.x () >= min && p.x () <= max);
  else
    return (p.y () >= min && p.y () <= max);
}
