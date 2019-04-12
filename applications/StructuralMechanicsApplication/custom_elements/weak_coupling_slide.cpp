// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:     BSD License
//           license: structural_mechanics_application/license.txt
//
//  Main authors: Klaus B. Sautter
//  node 1,2 connect line to run on
//  node 3 is slave node
//

// System includes

// External includes

// Project includes
#include "custom_elements/weak_coupling_slide.hpp"
#include "includes/define.h"
#include "structural_mechanics_application_variables.h"
#include "custom_utilities/structural_mechanics_element_utilities.h"
#include "includes/checks.h"


namespace Kratos {
WeakSlidingElement3D3N::WeakSlidingElement3D3N(IndexType NewId,
                                   GeometryType::Pointer pGeometry)
    : Element(NewId, pGeometry) {}

WeakSlidingElement3D3N::WeakSlidingElement3D3N(IndexType NewId,
                                   GeometryType::Pointer pGeometry,
                                   PropertiesType::Pointer pProperties)
    : Element(NewId, pGeometry, pProperties) {}

Element::Pointer
WeakSlidingElement3D3N::Create(IndexType NewId, NodesArrayType const& rThisNodes,
                         PropertiesType::Pointer pProperties) const
{
    const GeometryType& rGeom = GetGeometry();
    return Kratos::make_shared<WeakSlidingElement3D3N>(NewId, rGeom.Create(rThisNodes),
            pProperties);
}

Element::Pointer
WeakSlidingElement3D3N::Create(IndexType NewId, GeometryType::Pointer pGeom,
                         PropertiesType::Pointer pProperties) const
{
    return Kratos::make_shared<WeakSlidingElement3D3N>(NewId, pGeom,
            pProperties);
}

WeakSlidingElement3D3N::~WeakSlidingElement3D3N() {}

void WeakSlidingElement3D3N::EquationIdVector(EquationIdVectorType& rResult,
                                        ProcessInfo& rCurrentProcessInfo)
{

    if (rResult.size() != msLocalSize) {
        rResult.resize(msLocalSize);
    }

    for (int i = 0; i < msNumberOfNodes; ++i) {
        int index = i * 3;
        rResult[index] = GetGeometry()[i].GetDof(DISPLACEMENT_X).EquationId();
        rResult[index + 1] =
            GetGeometry()[i].GetDof(DISPLACEMENT_Y).EquationId();
        rResult[index + 2] =
            GetGeometry()[i].GetDof(DISPLACEMENT_Z).EquationId();
    }
}
void WeakSlidingElement3D3N::GetDofList(DofsVectorType& rElementalDofList,
                                  ProcessInfo& rCurrentProcessInfo)
{

    if (rElementalDofList.size() != msLocalSize) {
        rElementalDofList.resize(msLocalSize);
    }

    for (int i = 0; i < msNumberOfNodes; ++i) {
        int index = i * 3;
        rElementalDofList[index] = GetGeometry()[i].pGetDof(DISPLACEMENT_X);
        rElementalDofList[index + 1] =
            GetGeometry()[i].pGetDof(DISPLACEMENT_Y);
        rElementalDofList[index + 2] =
            GetGeometry()[i].pGetDof(DISPLACEMENT_Z);
    }
}

void WeakSlidingElement3D3N::Initialize()
{
    KRATOS_TRY
    KRATOS_CATCH("")
}

BoundedMatrix<double, WeakSlidingElement3D3N::msLocalSize,
WeakSlidingElement3D3N::msLocalSize>
WeakSlidingElement3D3N::CreateElementStiffnessMatrix(
    ProcessInfo& rCurrentProcessInfo)
{

    KRATOS_TRY
    BoundedMatrix<double, msLocalSize, msLocalSize> local_stiffness_matrix =
        ZeroMatrix(msLocalSize, msLocalSize);

    const double Xa = GetGeometry()[0].X0();
    const double Ya = GetGeometry()[0].Y0();
    const double Za = GetGeometry()[0].Z0();
    const double Xb = GetGeometry()[1].X0();
    const double Yb = GetGeometry()[1].Y0();
    const double Zb = GetGeometry()[1].Z0();
    const double Xc = GetGeometry()[2].X0();
    const double Yc = GetGeometry()[2].Y0();
    const double Zc = GetGeometry()[2].Z0();

    const double ua = GetGeometry()[0].FastGetSolutionStepValue(DISPLACEMENT_X, 0);
    const double va = GetGeometry()[0].FastGetSolutionStepValue(DISPLACEMENT_Y, 0);
    const double wa = GetGeometry()[0].FastGetSolutionStepValue(DISPLACEMENT_Z, 0);
    const double ub = GetGeometry()[1].FastGetSolutionStepValue(DISPLACEMENT_X, 0);
    const double vb = GetGeometry()[1].FastGetSolutionStepValue(DISPLACEMENT_Y, 0);
    const double wb = GetGeometry()[1].FastGetSolutionStepValue(DISPLACEMENT_Z, 0);
    const double uc = GetGeometry()[2].FastGetSolutionStepValue(DISPLACEMENT_X, 0);
    const double vc = GetGeometry()[2].FastGetSolutionStepValue(DISPLACEMENT_Y, 0);
    const double wc = GetGeometry()[2].FastGetSolutionStepValue(DISPLACEMENT_Z, 0);


    const double alpha = GetProperties()[YOUNG_MODULUS]; // simplified "spring stiffness"
    local_stiffness_matrix(0,0) = 1.0*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((Yb - Yc + vb - vc)*(2*Yb - 2*Yc + 2*vb - 2*vc) + (-2*Zb + 2*Zc - 2*wb + 2*wc)*(-Zb + Zc - wb + wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) - 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Xa + 4*Xb - 4*ua + 4*ub)*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(0,1) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Xa + 2*Xb - 2*ua + 2*ub)*(-4*Ya + 4*Yb - 4*va + 4*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(-Xb + Xc - ub + uc)*(2*Yb - 2*Yc + 2*vb - 2*vc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(0,2) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Xa + 2*Xb - 2*ua + 2*ub)*(-4*Za + 4*Zb - 4*wa + 4*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(Xb - Xc + ub - uc)*(-2*Zb + 2*Zc - 2*wb + 2*wc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(0,3) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-Ya + Yc - va + vc)*(2*Yb - 2*Yc + 2*vb - 2*vc) + (Za - Zc + wa - wc)*(-2*Zb + 2*Zc - 2*wb + 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Xa + 2*Xb - 2*ua + 2*ub)*(4*Xa - 4*Xb + 4*ua - 4*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(0,4) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb) + (Xa - Xc + ua - uc)*(2*Yb - 2*Yc + 2*vb - 2*vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Xa + 2*Xb - 2*ua + 2*ub)*(4*Ya - 4*Yb + 4*va - 4*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(0,5) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) + (-Xa + Xc - ua + uc)*(-2*Zb + 2*Zc - 2*wb + 2*wc) + 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Xa + 2*Xb - 2*ua + 2*ub)*(4*Za - 4*Zb + 4*wa - 4*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(0,6) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((Ya - Yb + va - vb)*(2*Yb - 2*Yc + 2*vb - 2*vc) + (-Za + Zb - wa + wb)*(-2*Zb + 2*Zc - 2*wb + 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(0,7) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (-Xa + Xb - ua + ub)*(2*Yb - 2*Yc + 2*vb - 2*vc) - 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(0,8) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) + (Xa - Xb + ua - ub)*(-2*Zb + 2*Zc - 2*wb + 2*wc) - 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(1,0) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Xa + 4*Xb - 4*ua + 4*ub)*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(-2*Xb + 2*Xc - 2*ub + 2*uc)*(Yb - Yc + vb - vc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(1,1) = 1.0*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Xb + 2*Xc - 2*ub + 2*uc)*(-Xb + Xc - ub + uc) + (Zb - Zc + wb - wc)*(2*Zb - 2*Zc + 2*wb - 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) - 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Ya + 4*Yb - 4*va + 4*vb)*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(1,2) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Ya + 2*Yb - 2*va + 2*vb)*(-4*Za + 4*Zb - 4*wa + 4*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(-Yb + Yc - vb + vc)*(2*Zb - 2*Zc + 2*wb - 2*wc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(1,3) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) - 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb) + (-2*Xb + 2*Xc - 2*ub + 2*uc)*(-Ya + Yc - va + vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(4*Xa - 4*Xb + 4*ua - 4*ub)*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(1,4) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((Xa - Xc + ua - uc)*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-Za + Zc - wa + wc)*(2*Zb - 2*Zc + 2*wb - 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Ya + 2*Yb - 2*va + 2*vb)*(4*Ya - 4*Yb + 4*va - 4*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(1,5) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb) + (Ya - Yc + va - vc)*(2*Zb - 2*Zc + 2*wb - 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Ya + 2*Yb - 2*va + 2*vb)*(4*Za - 4*Zb + 4*wa - 4*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(1,6) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb) + (-2*Xb + 2*Xc - 2*ub + 2*uc)*(Ya - Yb + va - vb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(1,7) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-Xa + Xb - ua + ub)*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (Za - Zb + wa - wb)*(2*Zb - 2*Zc + 2*wb - 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(1,8) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (-Ya + Yb - va + vb)*(2*Zb - 2*Zc + 2*wb - 2*wc) - 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(2,0) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Xa + 4*Xb - 4*ua + 4*ub)*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(2*Xb - 2*Xc + 2*ub - 2*uc)*(-Zb + Zc - wb + wc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(2,1) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Ya + 4*Yb - 4*va + 4*vb)*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(-2*Yb + 2*Yc - 2*vb + 2*vc)*(Zb - Zc + wb - wc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(2,2) = 1.0*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((Xb - Xc + ub - uc)*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-2*Yb + 2*Yc - 2*vb + 2*vc)*(-Yb + Yc - vb + vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) - 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Za + 4*Zb - 4*wa + 4*wb)*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(2,3) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb) + (2*Xb - 2*Xc + 2*ub - 2*uc)*(Za - Zc + wa - wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(4*Xa - 4*Xb + 4*ua - 4*ub)*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(2,4) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) - 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb) + (-2*Yb + 2*Yc - 2*vb + 2*vc)*(-Za + Zc - wa + wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(4*Ya - 4*Yb + 4*va - 4*vb)*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(2,5) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-Xa + Xc - ua + uc)*(2*Xb - 2*Xc + 2*ub - 2*uc) + (Ya - Yc + va - vc)*(-2*Yb + 2*Yc - 2*vb + 2*vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Za + 2*Zb - 2*wa + 2*wb)*(4*Za - 4*Zb + 4*wa - 4*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(2,6) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) + 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb) + (2*Xb - 2*Xc + 2*ub - 2*uc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(2,7) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb) + (-2*Yb + 2*Yc - 2*vb + 2*vc)*(Za - Zb + wa - wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(2,8) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((Xa - Xb + ua - ub)*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-Ya + Yb - va + vb)*(-2*Yb + 2*Yc - 2*vb + 2*vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(3,0) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Ya + 2*Yc - 2*va + 2*vc)*(Yb - Yc + vb - vc) + (2*Za - 2*Zc + 2*wa - 2*wc)*(-Zb + Zc - wb + wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Xa + 4*Xb - 4*ua + 4*ub)*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(3,1) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) - 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb) + (-Xb + Xc - ub + uc)*(-2*Ya + 2*Yc - 2*va + 2*vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Xa - 2*Xb + 2*ua - 2*ub)*(-4*Ya + 4*Yb - 4*va + 4*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(3,2) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb) + (Xb - Xc + ub - uc)*(2*Za - 2*Zc + 2*wa - 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Xa - 2*Xb + 2*ua - 2*ub)*(-4*Za + 4*Zb - 4*wa + 4*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(3,3) = 1.0*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Ya + 2*Yc - 2*va + 2*vc)*(-Ya + Yc - va + vc) + (Za - Zc + wa - wc)*(2*Za - 2*Zc + 2*wa - 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) - 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Xa - 2*Xb + 2*ua - 2*ub)*(4*Xa - 4*Xb + 4*ua - 4*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(3,4) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Xa - 2*Xb + 2*ua - 2*ub)*(4*Ya - 4*Yb + 4*va - 4*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(Xa - Xc + ua - uc)*(-2*Ya + 2*Yc - 2*va + 2*vc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(3,5) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Xa - 2*Xb + 2*ua - 2*ub)*(4*Za - 4*Zb + 4*wa - 4*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(-Xa + Xc - ua + uc)*(2*Za - 2*Zc + 2*wa - 2*wc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(3,6) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Ya + 2*Yc - 2*va + 2*vc)*(Ya - Yb + va - vb) + (-Za + Zb - wa + wb)*(2*Za - 2*Zc + 2*wa - 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(3,7) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-Xa + Xb - ua + ub)*(-2*Ya + 2*Yc - 2*va + 2*vc) - 2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(3,8) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) + (Xa - Xb + ua - ub)*(2*Za - 2*Zc + 2*wa - 2*wc) + 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(4,0) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb) + (2*Xa - 2*Xc + 2*ua - 2*uc)*(Yb - Yc + vb - vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Xa + 4*Xb - 4*ua + 4*ub)*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(4,1) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((2*Xa - 2*Xc + 2*ua - 2*uc)*(-Xb + Xc - ub + uc) + (-2*Za + 2*Zc - 2*wa + 2*wc)*(Zb - Zc + wb - wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Ya + 4*Yb - 4*va + 4*vb)*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(4,2) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) - 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb) + (-Yb + Yc - vb + vc)*(-2*Za + 2*Zc - 2*wa + 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Ya - 2*Yb + 2*va - 2*vb)*(-4*Za + 4*Zb - 4*wa + 4*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(4,3) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(4*Xa - 4*Xb + 4*ua - 4*ub)*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(2*Xa - 2*Xc + 2*ua - 2*uc)*(-Ya + Yc - va + vc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(4,4) = 1.0*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((Xa - Xc + ua - uc)*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-2*Za + 2*Zc - 2*wa + 2*wc)*(-Za + Zc - wa + wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) - 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Ya - 2*Yb + 2*va - 2*vb)*(4*Ya - 4*Yb + 4*va - 4*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(4,5) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Ya - 2*Yb + 2*va - 2*vb)*(4*Za - 4*Zb + 4*wa - 4*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(Ya - Yc + va - vc)*(-2*Za + 2*Zc - 2*wa + 2*wc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(4,6) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) - 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb) + (2*Xa - 2*Xc + 2*ua - 2*uc)*(Ya - Yb + va - vb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(4,7) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-Xa + Xb - ua + ub)*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-2*Za + 2*Zc - 2*wa + 2*wc)*(Za - Zb + wa - wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(4,8) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-Ya + Yb - va + vb)*(-2*Za + 2*Zc - 2*wa + 2*wc) - 2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(5,0) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Xa + 2*Xc - 2*ua + 2*uc)*(-Zb + Zc - wb + wc) - 2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) + 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Xa + 4*Xb - 4*ua + 4*ub)*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(5,1) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb) + (2*Ya - 2*Yc + 2*va - 2*vc)*(Zb - Zc + wb - wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Ya + 4*Yb - 4*va + 4*vb)*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(5,2) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Xa + 2*Xc - 2*ua + 2*uc)*(Xb - Xc + ub - uc) + (2*Ya - 2*Yc + 2*va - 2*vc)*(-Yb + Yc - vb + vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-4*Za + 4*Zb - 4*wa + 4*wb)*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(5,3) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(4*Xa - 4*Xb + 4*ua - 4*ub)*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(-2*Xa + 2*Xc - 2*ua + 2*uc)*(Za - Zc + wa - wc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(5,4) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(4*Ya - 4*Yb + 4*va - 4*vb)*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3) + 0.5*alpha*(2*Ya - 2*Yc + 2*va - 2*vc)*(-Za + Zc - wa + wc)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(5,5) = 1.0*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Xa + 2*Xc - 2*ua + 2*uc)*(-Xa + Xc - ua + uc) + (Ya - Yc + va - vc)*(2*Ya - 2*Yc + 2*va - 2*vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) - 1.0*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Za - 2*Zb + 2*wa - 2*wb)*(4*Za - 4*Zb + 4*wa - 4*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 3);
    local_stiffness_matrix(5,6) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Xa + 2*Xc - 2*ua + 2*uc)*(-Za + Zb - wa + wb) + 2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(5,7) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) - 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb) + (2*Ya - 2*Yc + 2*va - 2*vc)*(Za - Zb + wa - wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(5,8) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Xa + 2*Xc - 2*ua + 2*uc)*(Xa - Xb + ua - ub) + (-Ya + Yb - va + vb)*(2*Ya - 2*Yc + 2*va - 2*vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(6,0) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((2*Ya - 2*Yb + 2*va - 2*vb)*(Yb - Yc + vb - vc) + (-2*Za + 2*Zb - 2*wa + 2*wb)*(-Zb + Zc - wb + wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(6,1) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb) + (-Xb + Xc - ub + uc)*(2*Ya - 2*Yb + 2*va - 2*vb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(6,2) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) + 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb) + (Xb - Xc + ub - uc)*(-2*Za + 2*Zb - 2*wa + 2*wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(6,3) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-Ya + Yc - va + vc)*(2*Ya - 2*Yb + 2*va - 2*vb) + (-2*Za + 2*Zb - 2*wa + 2*wb)*(Za - Zc + wa - wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(6,4) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) - 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb) + (Xa - Xc + ua - uc)*(2*Ya - 2*Yb + 2*va - 2*vb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(6,5) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) + (-Xa + Xc - ua + uc)*(-2*Za + 2*Zb - 2*wa + 2*wb) - 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(6,6) = 0.5*alpha*((Ya - Yb + va - vb)*(2*Ya - 2*Yb + 2*va - 2*vb) + (-2*Za + 2*Zb - 2*wa + 2*wb)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(6,7) = 0.5*alpha*(-Xa + Xb - ua + ub)*(2*Ya - 2*Yb + 2*va - 2*vb)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(6,8) = 0.5*alpha*(Xa - Xb + ua - ub)*(-2*Za + 2*Zb - 2*wa + 2*wb)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(7,0) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Xa + 2*Xb - 2*ua + 2*ub)*(Yb - Yc + vb - vc) + 2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) - 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(7,1) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Xa + 2*Xb - 2*ua + 2*ub)*(-Xb + Xc - ub + uc) + (2*Za - 2*Zb + 2*wa - 2*wb)*(Zb - Zc + wb - wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(7,2) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb) + (-Yb + Yc - vb + vc)*(2*Za - 2*Zb + 2*wa - 2*wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(7,3) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Xa + 2*Xb - 2*ua + 2*ub)*(-Ya + Yc - va + vc) - 2*(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + 2*(Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(7,4) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Xa + 2*Xb - 2*ua + 2*ub)*(Xa - Xc + ua - uc) + (-Za + Zc - wa + wc)*(2*Za - 2*Zb + 2*wa - 2*wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(7,5) = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) - 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb) + (Ya - Yc + va - vc)*(2*Za - 2*Zb + 2*wa - 2*wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(7,6) = 0.5*alpha*(-2*Xa + 2*Xb - 2*ua + 2*ub)*(Ya - Yb + va - vb)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(7,7) = 0.5*alpha*((-2*Xa + 2*Xb - 2*ua + 2*ub)*(-Xa + Xb - ua + ub) + (Za - Zb + wa - wb)*(2*Za - 2*Zb + 2*wa - 2*wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(7,8) = 0.5*alpha*(-Ya + Yb - va + vb)*(2*Za - 2*Zb + 2*wa - 2*wb)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(8,0) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb) + (2*Xa - 2*Xb + 2*ua - 2*ub)*(-Zb + Zc - wb + wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(8,1) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Ya + 2*Yb - 2*va + 2*vb)*(Zb - Zc + wb - wc) + 2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) - 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(8,2) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((2*Xa - 2*Xb + 2*ua - 2*ub)*(Xb - Xc + ub - uc) + (-2*Ya + 2*Yb - 2*va + 2*vb)*(-Yb + Yc - vb + vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(8,3) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*(-2*(-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) + 2*(Xa - Xc + ua - uc)*(-Za + Zb - wa + wb) + (2*Xa - 2*Xb + 2*ua - 2*ub)*(Za - Zc + wa - wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(8,4) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-2*Ya + 2*Yb - 2*va + 2*vb)*(-Za + Zc - wa + wc) - 2*(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + 2*(Ya - Yc + va - vc)*(-Za + Zb - wa + wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(8,5) = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2) + 0.5*alpha*((-Xa + Xc - ua + uc)*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-2*Ya + 2*Yb - 2*va + 2*vb)*(Ya - Yc + va - vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(8,6) = 0.5*alpha*(2*Xa - 2*Xb + 2*ua - 2*ub)*(-Za + Zb - wa + wb)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(8,7) = 0.5*alpha*(-2*Ya + 2*Yb - 2*va + 2*vb)*(Za - Zb + wa - wb)/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    local_stiffness_matrix(8,8) = 0.5*alpha*((Xa - Xb + ua - ub)*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-2*Ya + 2*Yb - 2*va + 2*vb)*(-Ya + Yb - va + vb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));

    return local_stiffness_matrix;
    KRATOS_CATCH("")
}

void WeakSlidingElement3D3N::CalculateDampingMatrix(
    MatrixType& rDampingMatrix, ProcessInfo& rCurrentProcessInfo)
{
    StructuralMechanicsElementUtilities::CalculateRayleighDampingMatrix(
        *this,
        rDampingMatrix,
        rCurrentProcessInfo,
        msLocalSize);
}

void WeakSlidingElement3D3N::CalculateMassMatrix(
    MatrixType& rMassMatrix,
    ProcessInfo& rCurrentProcessInfo
)
{
    KRATOS_TRY

    // Compute lumped mass matrix
    VectorType temp_vector(msLocalSize);
    CalculateLumpedMassVector(temp_vector);

    // Clear matrix
    if (rMassMatrix.size1() != msLocalSize || rMassMatrix.size2() != msLocalSize) {
        rMassMatrix.resize(msLocalSize, msLocalSize, false);
    }
    rMassMatrix = ZeroMatrix(msLocalSize, msLocalSize);

    // Fill the matrix
    for (IndexType i = 0; i < msLocalSize; ++i) {
        rMassMatrix(i, i) = temp_vector[i];
    }

    KRATOS_CATCH("")
}

void WeakSlidingElement3D3N::GetValuesVector(Vector& rValues, int Step)
{

    KRATOS_TRY
    if (rValues.size() != msLocalSize) {
        rValues.resize(msLocalSize, false);
    }

    for (int i = 0; i < msNumberOfNodes; ++i) {
        int index = i * msDimension;
        const auto& disp =
            GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT, Step);

        rValues[index] = disp[0];
        rValues[index + 1] = disp[1];
        rValues[index + 2] = disp[2];
    }
    KRATOS_CATCH("")
}

void WeakSlidingElement3D3N::GetFirstDerivativesVector(Vector& rValues, int Step)
{

    KRATOS_TRY
    if (rValues.size() != msLocalSize) {
        rValues.resize(msLocalSize, false);
    }

    for (int i = 0; i < msNumberOfNodes; ++i) {
        int index = i * msDimension;
        const auto& vel =
            GetGeometry()[i].FastGetSolutionStepValue(VELOCITY, Step);

        rValues[index] = vel[0];
        rValues[index + 1] = vel[1];
        rValues[index + 2] = vel[2];
    }
    KRATOS_CATCH("")
}

void WeakSlidingElement3D3N::GetSecondDerivativesVector(Vector& rValues, int Step)
{

    KRATOS_TRY
    if (rValues.size() != msLocalSize) {
        rValues.resize(msLocalSize, false);
    }

    for (int i = 0; i < msNumberOfNodes; ++i) {
        int index = i * msDimension;
        const auto& acc =
            GetGeometry()[i].FastGetSolutionStepValue(ACCELERATION, Step);

        rValues[index] = acc[0];
        rValues[index + 1] = acc[1];
        rValues[index + 2] = acc[2];
    }

    KRATOS_CATCH("")
}

void WeakSlidingElement3D3N::CalculateLocalSystem(MatrixType& rLeftHandSideMatrix,
        VectorType& rRightHandSideVector,
        ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;
    CalculateRightHandSide(rRightHandSideVector,rCurrentProcessInfo);
    CalculateLeftHandSide(rLeftHandSideMatrix,rCurrentProcessInfo);
    KRATOS_CATCH("")
}

void WeakSlidingElement3D3N::CalculateRightHandSide(
    VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo)
{

    KRATOS_TRY
    rRightHandSideVector = ZeroVector(msLocalSize);

    BoundedVector<double, msLocalSize> internal_forces = ZeroVector(msLocalSize);

    const double Xa = GetGeometry()[0].X0();
    const double Ya = GetGeometry()[0].Y0();
    const double Za = GetGeometry()[0].Z0();
    const double Xb = GetGeometry()[1].X0();
    const double Yb = GetGeometry()[1].Y0();
    const double Zb = GetGeometry()[1].Z0();
    const double Xc = GetGeometry()[2].X0();
    const double Yc = GetGeometry()[2].Y0();
    const double Zc = GetGeometry()[2].Z0();

    const double ua = GetGeometry()[0].FastGetSolutionStepValue(DISPLACEMENT_X, 0);
    const double va = GetGeometry()[0].FastGetSolutionStepValue(DISPLACEMENT_Y, 0);
    const double wa = GetGeometry()[0].FastGetSolutionStepValue(DISPLACEMENT_Z, 0);
    const double ub = GetGeometry()[1].FastGetSolutionStepValue(DISPLACEMENT_X, 0);
    const double vb = GetGeometry()[1].FastGetSolutionStepValue(DISPLACEMENT_Y, 0);
    const double wb = GetGeometry()[1].FastGetSolutionStepValue(DISPLACEMENT_Z, 0);
    const double uc = GetGeometry()[2].FastGetSolutionStepValue(DISPLACEMENT_X, 0);
    const double vc = GetGeometry()[2].FastGetSolutionStepValue(DISPLACEMENT_Y, 0);
    const double wc = GetGeometry()[2].FastGetSolutionStepValue(DISPLACEMENT_Z, 0);


    const double alpha = GetProperties()[YOUNG_MODULUS]; // simplified "spring stiffness"

    internal_forces[0] = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Yb - 2*Yc + 2*vb - 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Zb + 2*Zc - 2*wb + 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Xa + 2*Xb - 2*ua + 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2);
    internal_forces[1] = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xb + 2*Xc - 2*ub + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Zb - 2*Zc + 2*wb - 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Ya + 2*Yb - 2*va + 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2);
    internal_forces[2] = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xb - 2*Xc + 2*ub - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Yb + 2*Yc - 2*vb + 2*vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(-2*Za + 2*Zb - 2*wa + 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2);
    internal_forces[3] = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Ya + 2*Yc - 2*va + 2*vc) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zc + 2*wa - 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Xa - 2*Xb + 2*ua - 2*ub)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2);
    internal_forces[4] = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Xa - 2*Xc + 2*ua - 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zc - 2*wa + 2*wc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Ya - 2*Yb + 2*va - 2*vb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2);
    internal_forces[5] = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Xa + 2*Xc - 2*ua + 2*uc) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Ya - 2*Yc + 2*va - 2*vc))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2)) + 0.5*alpha*(std::pow(-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb), 2) + std::pow((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb), 2) + std::pow(-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb), 2))*(2*Za - 2*Zb + 2*wa - 2*wb)/std::pow(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2), 2);
    internal_forces[6] = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(2*Ya - 2*Yb + 2*va - 2*vb) + ((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(-2*Za + 2*Zb - 2*wa + 2*wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    internal_forces[7] = 0.5*alpha*((-(-Xa + Xb - ua + ub)*(Ya - Yc + va - vc) + (Xa - Xc + ua - uc)*(-Ya + Yb - va + vb))*(-2*Xa + 2*Xb - 2*ua + 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(2*Za - 2*Zb + 2*wa - 2*wb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));
    internal_forces[8] = 0.5*alpha*(((-Xa + Xb - ua + ub)*(Za - Zc + wa - wc) - (Xa - Xc + ua - uc)*(-Za + Zb - wa + wb))*(2*Xa - 2*Xb + 2*ua - 2*ub) + (-(-Ya + Yb - va + vb)*(Za - Zc + wa - wc) + (Ya - Yc + va - vc)*(-Za + Zb - wa + wb))*(-2*Ya + 2*Yb - 2*va + 2*vb))/(std::pow(-Xa + Xb - ua + ub, 2) + std::pow(-Ya + Yb - va + vb, 2) + std::pow(-Za + Zb - wa + wb, 2));


    noalias(rRightHandSideVector) -= internal_forces;
    KRATOS_CATCH("")
}

void WeakSlidingElement3D3N::CalculateLeftHandSide(MatrixType& rLeftHandSideMatrix,
        ProcessInfo& rCurrentProcessInfo)
{

    KRATOS_TRY;
    // resizing the matrices + create memory for LHS
    rLeftHandSideMatrix = ZeroMatrix(msLocalSize, msLocalSize);
    // creating LHS
    noalias(rLeftHandSideMatrix) =
        CreateElementStiffnessMatrix(rCurrentProcessInfo);
    KRATOS_CATCH("")
}

int WeakSlidingElement3D3N::Check(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY
    const double numerical_limit = std::numeric_limits<double>::epsilon();
    const SizeType number_of_nodes = GetGeometry().size();
    const SizeType dimension = GetGeometry().WorkingSpaceDimension();

    if (dimension != msDimension ||number_of_nodes != msNumberOfNodes) {
        KRATOS_ERROR << "The truss element works only in 3D and with 2 noded elements" << std::endl;
    }
    // verify that the variables are correctly initialized
    KRATOS_CHECK_VARIABLE_KEY(DISPLACEMENT);
    KRATOS_CHECK_VARIABLE_KEY(VELOCITY);
    KRATOS_CHECK_VARIABLE_KEY(ACCELERATION);
    KRATOS_CHECK_VARIABLE_KEY(DENSITY);
    KRATOS_CHECK_VARIABLE_KEY(CROSS_AREA);

    // Check that the element's nodes contain all required SolutionStepData and Degrees of freedom
    for (IndexType i = 0; i < number_of_nodes; ++i) {
        NodeType& rnode = GetGeometry()[i];
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DISPLACEMENT, rnode);

        KRATOS_CHECK_DOF_IN_NODE(DISPLACEMENT_X, rnode);
        KRATOS_CHECK_DOF_IN_NODE(DISPLACEMENT_Y, rnode);
        KRATOS_CHECK_DOF_IN_NODE(DISPLACEMENT_Z, rnode);
    }

    if (GetProperties().Has(CROSS_AREA) == false ||
            GetProperties()[CROSS_AREA] <= numerical_limit) {
        KRATOS_ERROR << "CROSS_AREA not provided for this element" << Id()
                     << std::endl;
    }

    if (GetProperties().Has(YOUNG_MODULUS) == false ||
            GetProperties()[YOUNG_MODULUS] <= numerical_limit) {
        KRATOS_ERROR << "YOUNG_MODULUS not provided for this element" << Id()
                     << std::endl;
    }
    if (GetProperties().Has(DENSITY) == false) {
        KRATOS_ERROR << "DENSITY not provided for this element" << Id()
                     << std::endl;
    }

    KRATOS_ERROR_IF(StructuralMechanicsElementUtilities::CalculateReferenceLength3D2N(*this)
                    < std::numeric_limits<double>::epsilon())
            << "Element #" << Id() << " has a length of zero!" << std::endl;

    return 0;

    KRATOS_CATCH("")
}

void WeakSlidingElement3D3N::AddExplicitContribution(
    const VectorType& rRHSVector,
    const Variable<VectorType>& rRHSVariable,
    Variable<double >& rDestinationVariable,
    const ProcessInfo& rCurrentProcessInfo
)
{
    KRATOS_TRY;

    auto& r_geom = GetGeometry();

    if (rDestinationVariable == NODAL_MASS) {
        VectorType element_mass_vector(msLocalSize);
        CalculateLumpedMassVector(element_mass_vector);

        for (SizeType i = 0; i < msNumberOfNodes; ++i) {
            double& r_nodal_mass = r_geom[i].GetValue(NODAL_MASS);
            int index = i * msDimension;

            #pragma omp atomic
            r_nodal_mass += element_mass_vector(index);
        }
    }

    KRATOS_CATCH("")
}

void WeakSlidingElement3D3N::AddExplicitContribution(
    const VectorType& rRHSVector, const Variable<VectorType>& rRHSVariable,
    Variable<array_1d<double, 3>>& rDestinationVariable,
    const ProcessInfo& rCurrentProcessInfo
)
{
    KRATOS_TRY;

    if (rRHSVariable == RESIDUAL_VECTOR && rDestinationVariable == FORCE_RESIDUAL) {

        BoundedVector<double, msLocalSize> damping_residual_contribution = ZeroVector(msLocalSize);
        Vector current_nodal_velocities = ZeroVector(msLocalSize);
        GetFirstDerivativesVector(current_nodal_velocities);
        Matrix damping_matrix;
        ProcessInfo temp_process_information; // cant pass const ProcessInfo
        CalculateDampingMatrix(damping_matrix, temp_process_information);
        // current residual contribution due to damping
        noalias(damping_residual_contribution) = prod(damping_matrix, current_nodal_velocities);

        for (size_t i = 0; i < msNumberOfNodes; ++i) {
            size_t index = msDimension * i;
            array_1d<double, 3>& r_force_residual = GetGeometry()[i].FastGetSolutionStepValue(FORCE_RESIDUAL);
            for (size_t j = 0; j < msDimension; ++j) {
                #pragma omp atomic
                r_force_residual[j] += rRHSVector[index + j] - damping_residual_contribution[index + j];
            }
        }
    } else if (rDestinationVariable == NODAL_INERTIA) {

        // Getting the vector mass
        VectorType mass_vector(msLocalSize);
        CalculateLumpedMassVector(mass_vector);

        for (int i = 0; i < msNumberOfNodes; ++i) {
            double& r_nodal_mass = GetGeometry()[i].GetValue(NODAL_MASS);
            array_1d<double, msDimension>& r_nodal_inertia = GetGeometry()[i].GetValue(NODAL_INERTIA);
            int index = i * msDimension;

            #pragma omp atomic
            r_nodal_mass += mass_vector[index];

            for (int k = 0; k < msDimension; ++k) {
                #pragma omp atomic
                r_nodal_inertia[k] += 0.0;
            }
        }
    }

    KRATOS_CATCH("")
}
void WeakSlidingElement3D3N::save(Serializer &rSerializer) const {
  KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Element);
}
void WeakSlidingElement3D3N::load(Serializer& rSerializer)
{
    KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Element);
}


void WeakSlidingElement3D3N::CalculateLumpedMassVector(VectorType& rMassVector)
{
    KRATOS_TRY

    // Clear matrix
    if (rMassVector.size() != msLocalSize) {
        rMassVector.resize(msLocalSize, false);
    }

    const double A = GetProperties()[CROSS_AREA];
    const double L = StructuralMechanicsElementUtilities::CalculateReferenceLength3D2N(*this);
    const double rho = GetProperties()[DENSITY];

    //const double total_mass = A * L * rho;
    const double total_mass = 0.0;

    for (int i = 0; i < msNumberOfNodes; ++i) {
        for (int j = 0; j < msDimension; ++j) {
            int index = i * msDimension + j;

            rMassVector[index] = total_mass * 0.50;
        }
    }

    KRATOS_CATCH("")
}



} // namespace Kratos.
