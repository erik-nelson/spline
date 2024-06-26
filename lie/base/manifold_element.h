#pragma once

#include <array>

#include "lie/base/constants.h"

namespace mana {

// Traits template for a manifold.
template <typename Derived>
struct ManifoldTraits {};

// Base CRTP class for an element of a (differentiable) manifold. Manifolds are
// expressed in an extrinsic fashion, i.e. they are assumed to be embedded
// within a larger ambient (vector) space.
//
// Derived classes must implement these methods.
// - static Element FromPointImpl(const EmbeddingPoint& point);
// - static Element ProjectImpl(const EmbeddingPoint& point);
// - static bool IsValidImpl(const EmbeddingPoint& point, Scalar tolerance);
// - EmbeddingPoint PointImpl() const;
// - std::array<TangentVector, Dimension> TangentSpaceBasisImpl() const;
// - Scalar DistanceToImpl(const Element& rhs) const;
// - Element InterpolateImpl(const Element& rhs, Scalar fraction) const;
template <typename Derived>
class ManifoldElement {
 public:
  // An element of the manifold (identical to `Derived`).
  using Element = typename ManifoldTraits<Derived>::Element;
  // The scalar type used to represent distances.
  using Scalar = typename ManifoldTraits<Derived>::Scalar;
  // The chart type (see `ManifoldChart` below).
  using Chart = typename ManifoldTraits<Derived>::Chart;
  // The geodesic type (see `ManifoldGeodesic` below).
  using Geodesic = typename ManifoldTraits<Derived>::Geodesic;
  // A vector in a tangent space of this manifold.
  using TangentVector = typename ManifoldTraits<Derived>::TangentVector;
  // A point in the embedding space that this manifold lies in. Not necessarily
  // a vector. For instance, points in the embedding space of 3D rotations SO(3)
  // are 3x3 matrices.
  using EmbeddingPoint = typename ManifoldTraits<Derived>::EmbeddingPoint;
  // The dimension of the manifold.
  static constexpr int Dimension = ManifoldTraits<Derived>::Dimension;
  // The dimension of the embedding space.
  static constexpr int EmbeddingDimension =
      ManifoldTraits<Derived>::EmbeddingDimension;

  // Construct from a point in the embedding space. Assumes the provided point
  // lies ont he manifold.
  static Element FromPoint(const EmbeddingPoint& point);

  // Project a point from the embedding space onto the manifold. For example, an
  // SO(3) derived manifold element type might implement this by projecting a
  // 3x3 matrix onto {R | R'R=I, det(R)=1}, the space of valid rotation
  // matrices.
  static EmbeddingPoint Project(const EmbeddingPoint& point);

  // Checks if this point in embedding space lies on the manifold.
  static bool IsValid(const EmbeddingPoint& point,
                      Scalar tolerance = Constants<Scalar>::kEpsilon);

  // Return this element's point in the underlying embedding space.
  EmbeddingPoint Point() const;

#if 0
  // Build a chart at this point on the manifold.
  Chart LocalChart() const { return Chart(*this); }

  // Returns a basis for the tangent space at this point on the manifold.
  std::array<TangentVector, Dimension> TangentSpaceBasis() const {
    return derived().TangentSpaceBasisImpl();
  }
#endif

  // Builds a geodesic curve parameterized between two points on the manifold.
  Geodesic GeodesicTo(const Element& rhs) const;

  // Compute the distance between two points on the manifold.
  Scalar DistanceTo(const Element& rhs) const;

  // Interpolate along the path from this manifold element to `rhs`, following
  // the geodesic. The provided fraction should be in [0, 1] for points along
  // the geodesic, and outside of that range to perform extrapolation instead.
  Element Interpolate(const Element& rhs, Scalar fraction) const;

  // Check if this manifold element is roughly equal to `rhs`.
  bool EqualTo(const Element& rhs,
               Scalar tolerance = Constants<Scalar>::kEpsilon) const;

  // Equality checking.
  bool operator==(const Element& rhs) const;

  // Inequality checking.
  bool operator!=(const Element& rhs) const;

 private:
  // CRTP helpers.
  Derived& derived() { return static_cast<Derived&>(*this); }
  const Derived& derived() const { return static_cast<const Derived&>(*this); }
};

// Class representing a chart on a manifold, mapping from the manifold to its
// tangent space.
template <typename Derived>
class ManifoldChart {
 public:
  using Element = typename ManifoldTraits<Derived>::Element;
  using TangentVector = typename ManifoldTraits<Derived>::TangentVector;

  // Construct from origin point on the manifold.
  explicit ManifoldChart(Element origin);

  // Map an element on the manifold to a tangent vector using the chart's
  // forward map.
  TangentVector ToTangent(const Element& rhs) const;

  // Map a tangent vector to an element on the manifold using the chart's
  // reverse map.
  Element ToManifold(const TangentVector& rhs) const;

 private:
  // The point forming the origin of this chart. The zero tangent vector is
  // mapped to this point on the manifold.
  Element origin_;
};

template <typename Derived>
class ManifoldGeodesic {
 public:
  using Scalar = typename ManifoldTraits<Derived>::Scalar;
  using Element = typename ManifoldTraits<Derived>::Element;

  // Construct from start and end points.
  ManifoldGeodesic(Element beg, Element end);

  // Return the geodesic's begin point.
  const Element& beg() const;

  // Return the geodesic's end point.
  const Element& end() const;

  // Interpolate along the geodesic at the provided fraction. Values in [0, 1]
  // perform true interpolation, values outside of this range perform
  // extrapolation.
  Element Interpolate(Scalar fraction) const;

  // Return the length of this geodesic.
  Scalar Length() const;

 private:
  // The geodesic's start and end points.
  Element beg_, end_;
};

template <typename Derived>
/*static*/ typename ManifoldElement<Derived>::Element
ManifoldElement<Derived>::FromPoint(const EmbeddingPoint& point) {
  return Derived::FromPointImpl(point);
}

template <typename Derived>
/*static*/ typename ManifoldElement<Derived>::EmbeddingPoint
ManifoldElement<Derived>::Project(const EmbeddingPoint& point) {
  return Derived::ProjectImpl(point);
}

template <typename Derived>
/*static*/ bool ManifoldElement<Derived>::IsValid(const EmbeddingPoint& point,
                                                  Scalar tolerance) {
  return Derived::IsValidImpl(point, tolerance);
}

template <typename Derived>
typename ManifoldElement<Derived>::EmbeddingPoint
ManifoldElement<Derived>::Point() const {
  return derived().PointImpl();
}

template <typename Derived>
typename ManifoldElement<Derived>::Geodesic
ManifoldElement<Derived>::GeodesicTo(const Element& rhs) const {
  return Geodesic(derived(), rhs);
}

template <typename Derived>
typename ManifoldElement<Derived>::Scalar ManifoldElement<Derived>::DistanceTo(
    const Element& rhs) const {
  return derived().DistanceToImpl(rhs);
}

template <typename Derived>
typename ManifoldElement<Derived>::Element
ManifoldElement<Derived>::Interpolate(const Element& rhs,
                                      Scalar fraction) const {
  return derived().InterpolateImpl(rhs, fraction);
}

template <typename Derived>
bool ManifoldElement<Derived>::EqualTo(const Element& rhs,
                                       Scalar tolerance) const {
  return DistanceTo(rhs) < tolerance;
}

template <typename Derived>
bool ManifoldElement<Derived>::operator==(const Element& rhs) const {
  return EqualTo(rhs);
}

template <typename Derived>
bool ManifoldElement<Derived>::operator!=(const Element& rhs) const {
  return !(*this == rhs);
}

template <typename Derived>
ManifoldChart<Derived>::ManifoldChart(Element origin)
    : origin_(std::move(origin)) {}

template <typename Derived>
typename ManifoldChart<Derived>::TangentVector
ManifoldChart<Derived>::ToTangent(const Element& rhs) const {
  // TODO(erik): implement.
  return {};
}

template <typename Derived>
typename ManifoldChart<Derived>::Element ManifoldChart<Derived>::ToManifold(
    const TangentVector& rhs) const {
  // TODO(erik): implement.
  return {};
}

template <typename Derived>
ManifoldGeodesic<Derived>::ManifoldGeodesic(Element beg, Element end)
    : beg_(std::move(beg)), end_(std::move(end)) {}

template <typename Derived>
const typename ManifoldGeodesic<Derived>::Element&
ManifoldGeodesic<Derived>::beg() const {
  return beg_;
}

template <typename Derived>
const typename ManifoldGeodesic<Derived>::Element&
ManifoldGeodesic<Derived>::end() const {
  return end_;
}

template <typename Derived>
typename ManifoldGeodesic<Derived>::Element
ManifoldGeodesic<Derived>::Interpolate(Scalar fraction) const {
  return beg_.Interpolate(end_, fraction);
}

template <typename Derived>
typename ManifoldGeodesic<Derived>::Scalar ManifoldGeodesic<Derived>::Length()
    const {
  return beg_.DistanceTo(end_);
}

}  // namespace mana