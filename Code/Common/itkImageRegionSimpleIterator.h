/*==========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkImageRegionSimpleIterator.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.
  
==========================================================================*/
#ifndef __itkImageRegionSimpleIterator_h
#define __itkImageRegionSimpleIterator_h

#include "itkImageIteratorWithIndex.h"

namespace itk
{

/**
 * \class ImageRegionSimpleIterator
 * \brief Multi-dimensional image iterator which only walks a region.
 * 
 * ImageRegionSimpleIterator is a templated class to represent a multi-dimensional
 * iterator. ImageRegionSimpleIterator is templated over the dimension of the image
 * and the data type of the image. ImageRegionSimpleIterator is constrained to walk
 * only within the specified region.
 *
 * ImageRegionSimpleIterator is a multi-dimensional iterator, requiring more
 * information be specified before the iterator can be used than conventional
 * iterators. Whereas the std::vector::iterator from the STL only needs to be
 * passed a pointer to establish the iterator, the multi-dimensional image
 * iterator needs a pointer, the size of the buffer, the size of the region,
 * the start index of the buffer, and the start index of the region. To gain
 * access to this information, ImageRegionSimpleIterator holds a reference to the
 * image over which it is traversing.
 *
 * ImageRegionSimpleIterator assumes a particular layout of the image data. The
 * is arranged in a 1D array as if it were [][][][slice][row][col] with
 * Index[0] = col, Index[1] = row, Index[2] = slice, etc.
 *
 * operator++ provides a simple syntax for walking around a region of
 * a multidimensional image. operator++ iterates across a row, constraining
 * the movement to within a region of image. When the iterator reaches
 * the boundary of the region along a row, the iterator automatically
 * wraps to the next row, starting at the first pixel in the row that is
 * part of the region. This allows for simple processing loops of the form:
 *
 *      for (it = image->RegionBegin(); it != image->RegionEnd(); ++it)
 *         {
 *         *it += 100.0;
 *         }
 *
 */
template<typename TPixel, unsigned int VImageDimension=2>
class ImageRegionSimpleIterator : public ImageIteratorWithIndex<TPixel, VImageDimension>
{
public:
  /**
   * Standard "Self" typedef.
   */
  typedef ImageRegionSimpleIterator Self;

  /** 
   * Index typedef support. While this was already typdef'ed in the superclass
   * it needs to be redone here for this subclass to compile properly with gcc.
   * Note that we have to rescope Index back to itk::Index to that is it not
   * confused with ImageIterator::Index.
   */
  typedef itk::Index<VImageDimension> Index;

  /**
   * Image typedef support. While this was already typdef'ed in the superclass
   * it needs to be redone here for this subclass to compile properly with gcc.
   * Note that we have to rescope Index back to itk::Index to that is it not
   * confused with ImageIterator::Index.
   */
  typedef itk::Image<TPixel, VImageDimension> Image;

  /**
   * Default constructor. Needed since we provide a cast constructor.
   */
  ImageRegionSimpleIterator() : ImageIteratorWithIndex<TPixel, VImageDimension>() {}
  
  /**
   * Constructor establishes an iterator to walk a particular image and a
   * particular region of that image.
   */
  ImageRegionSimpleIterator(const SmartPointer<Image> &ptr,
                      const Index &start,
                      const unsigned long size[VImageDimension])
    : ImageIteratorWithIndex<TPixel, VImageDimension>(ptr, start, size) {}

  /**
   * Constructor that can be used to cast from an ImageIterator to an
   * ImageRegionSimpleIterator. Many routines return an ImageIterator but for a
   * particular task, you may want an ImageRegionSimpleIterator.  Rather than
   * provide overloaded APIs that return different types of Iterators, itk
   * returns ImageIterators and uses constructors to cast from an
   * ImageIterator to a ImageRegionSimpleIterator.
   */
  ImageRegionSimpleIterator( const ImageIteratorWithIndex<TPixel, VImageDimension> &it)
    { this->ImageIteratorWithIndex<TPixel, VImageDimension>::operator=(it); }

  
  /**
   * Increment (prefix) the fastest moving dimension of the iterator's index.
   * This operator will constrain the iterator within the region (i.e. the
   * iterator will automatically wrap from the end of the row of the region
   * to the beginning of the next row of the region) up until the iterator
   * tries to moves past the last pixel of the region.  Here, the iterator
   * will be set to be one pixel past the end of the region.
   * \sa operator++(int)
   */
  Self &
  operator++()
  {

    m_Remaining = false;
    for( unsigned int in=0; in<VImageDimension; in++ )
    {
      m_PositionIndex[ in  ]++;
      if( m_PositionIndex[ in ] < m_EndIndex[ in ] )
      {
        m_Position ++;
        m_Remaining = true;
        break;
      }
      else 
      {
        m_PositionIndex[ in ] = m_BeginIndex[ in ]; 
      }
    }

    if( !m_Remaining ) // It will not advance here otherwise
    {
      m_Position = m_End;
    }

    return *this;
  }

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkImageRegionSimpleIterator.txx"
#endif

#endif 
