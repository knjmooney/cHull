
using namespace std;

// This function manages the cycling problem - since set is cyclical, we have to handle the case that to<from
template<class T> void removeInsert (	std::vector < T > &set,
					T  toInsert,
					unsigned int from,unsigned int to) {

  unsigned int toMinusOne;
  if (to==0) {
    toMinusOne=set.size()-1;
  } else {
    toMinusOne=to-1;
  }
  cout << "ConvexHull::removeInsert from=" << from << " to=" << to << endl;
  cout << "ConvexHull::removeInsert toMinusOne=" << toMinusOne << " from+1=" << from+1 << endl;

  // TODO - What if "to" is 0?????
  if ( (toMinusOne)<(from+1) ) {
    // First, remove at the end
    set.erase (set.begin()+(from+1),set.begin()+set.size());
    // Then, remove at the beginning
    set.erase (set.begin(),set.begin()+(to));
    // Then, push_back at the end
    try {
      set.push_back(toInsert);
    } catch (std::bad_alloc const&) {
      cout << "ConvexHull::removeInsert set memory allocation fail!" << endl;	exit(1);
    }
  } else {	// Easy case - remove from (from+1) to (to-1), then insert in (from+1)
    if ( (toMinusOne)==(from+1) ) {
      set.erase (set.begin()+(from+1));
    } else if (to==0) {	// We have to erase from (from+1) onwards
      set.erase (set.begin()+(from+1),set.begin()+set.size());
    } else {
      set.erase (set.begin()+(from+1),set.begin()+(toMinusOne));
    }
    try {
      set.insert(set.begin()+(from+1),toInsert);
    } catch (std::bad_alloc const&) {
      cout << "ConvexHull::removeInsert set memory allocation fail!" << endl;	exit(1);
    }
  }
}
