



class IceList
{
    public:
        Point3D point;
        IceList* next;

        IceList(float x, float y, float z)
            : point{x, y, z}, next(nullptr) {}
};


