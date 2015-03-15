void Report(const Antibody a[CLASS_COUNT][MAX_ANTIBODIES])
{
        ofstream rep("champions.rep", ios::trunc);
        rep << setw(7) << "Class";
        rep << setw(3) << "#";
        rep << setw(9) << "Fitness";
        rep << setw(11) << "Attribute";
        rep << setw(8) << "Offset";
        rep << setw(20) << "Accuracy Per Class";
        rep << endl;
        rep << setw(7) << "-----";
        rep << setw(3) << "-";
        rep << setw(9) << "-------";
        rep << setw(11) << "---------";
        rep << setw(8) << "------";
        rep << setw(20) << "------------------";
        rep << endl;
        rep.close();

        return;
}
