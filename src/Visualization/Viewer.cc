#include <cmath>

#include <QFileInfo>

#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkCornerAnnotation.h>

#include <Visualization/Viewer.h>

#include <Data/Mesh.h>

//-----------------------------------------------------------------------------
Viewer::Viewer()
{

  this->renderer_ = vtkSmartPointer<vtkRenderer>::New();
  this->tile_layout_width_ = 4;
  this->tile_layout_height_ = 4;
  this->camera_ = this->renderer_->GetActiveCamera();
  this->start_row_ = 0;
  this->first_draw_ = true;
}

//-----------------------------------------------------------------------------
Viewer::~Viewer()
{}

//-----------------------------------------------------------------------------
void Viewer::set_interactor( vtkRenderWindowInteractor* interactor )
{
  this->interactor_ = interactor;
}

//-----------------------------------------------------------------------------
void Viewer::insert_mesh_into_view( vtkSmartPointer<vtkPolyData> poly_data, int position, int id, QString note )
{
  if ( position >= this->renderers_.size() )
  {
    return;
  }

  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();

#if VTK_MAJOR_VERSION <= 5
  mapper->SetInput( poly_data );
#else
  mapper->SetInputData( poly_data );
#endif
  actor->SetMapper( mapper );
  actor->GetProperty()->SetDiffuseColor( 1, 191.0 / 255.0, 0 );
  actor->GetProperty()->SetSpecular( 0.2 );
  actor->GetProperty()->SetSpecularPower( 15 );
  mapper->ScalarVisibilityOff();

  vtkSmartPointer<vtkRenderer> ren = this->renderers_[position];

  ren->RemoveAllViewProps();
  ren->AddActor( actor );

  if ( this->first_draw_ )
  {
    this->first_draw_ = false;
    ren->ResetCamera();
  }

  vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation =
    vtkSmartPointer<vtkCornerAnnotation>::New();
  cornerAnnotation->SetLinearFontScaleFactor( 2 );
  cornerAnnotation->SetNonlinearFontScaleFactor( 1 );
  cornerAnnotation->SetMaximumFontSize( 16 );
  cornerAnnotation->SetText( 2, QString::number( id ).toStdString().c_str() );
  cornerAnnotation->SetText( 0, note.toStdString().c_str() );
  cornerAnnotation->GetTextProperty()->SetColor( 0.50, 0.5, 0.5 );

  ren->AddViewProp( cornerAnnotation );
}

//-----------------------------------------------------------------------------
void Viewer::clear_renderers()
{
  for ( int i = 0; i < this->renderers_.size(); i++ )
  {
    this->renderers_[i]->RemoveAllViewProps();
  }
}

//-----------------------------------------------------------------------------
void Viewer::display_meshes()
{
  this->clear_renderers();

  // skip based on scrollbar
  int start_mesh = this->start_row_ * this->tile_layout_width_;

  int position = 0;
  for ( int i = start_mesh; i < this->meshes_.size(); i++ )
  {
    int id = i + 1;
    this->insert_mesh_into_view( this->meshes_[i]->get_poly_data(), position, id, this->meshes_[i]->get_filename() );
    position++;
  }
  this->render_window_->Render();
}

//-----------------------------------------------------------------------------
void Viewer::set_render_window( vtkRenderWindow* renderWindow )
{
  this->render_window_ = renderWindow;
  this->render_window_->AddRenderer( this->renderer_ );

  this->setup_renderers();
}

//-----------------------------------------------------------------------------
void Viewer::setup_renderers()
{
  for ( int i = 0; i < this->renderers_.size(); i++ )
  {
    this->render_window_->RemoveRenderer( this->renderers_[i] );
  }

  this->renderers_.clear();

  int* size = this->render_window_->GetSize();

  int width = this->tile_layout_width_;
  int height = this->tile_layout_height_;
  int total = width * height;

  float margin = 0.005;

  float tile_height = ( 1.0f - ( margin * ( height + 1 ) ) ) / height;
  float tile_width = ( 1.0f - ( margin * ( width + 1 ) ) ) / width;

  float step_x = tile_width + margin;
  float step_y = tile_height + margin;

  for ( int y = 0; y < height; y++ )
  {
    for ( int x = 0; x < width; x++ )
    {
      int i = ( y * width ) + x;

      vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
      ren->SetActiveCamera( this->camera_ );
      this->renderers_.push_back( ren );

      double viewport[4] = {0.0, 0.0, 0.0, 0.0};

      // horizontal
      viewport[0] = margin + ( x * step_x );
      viewport[2] = viewport[0] + tile_width;

      // vertical
      viewport[1] = margin + ( ( ( height - 1 ) - y ) * step_y );
      viewport[3] = viewport[1] + tile_height;

      ren->SetViewport( viewport );

      double color = 0.2f;

      ren->SetBackground( color, color, color );

      this->render_window_->AddRenderer( ren );
    }
  }

  this->render_window_->Render();
}

//-----------------------------------------------------------------------------
void Viewer::set_tile_layout( int width, int height )
{
  this->tile_layout_width_ = width;
  this->tile_layout_height_ = height;

  this->setup_renderers();
  this->display_meshes();
}

//-----------------------------------------------------------------------------
int Viewer::get_num_rows()
{
  return std::ceil( (float)this->meshes_.size() / (float)this->tile_layout_width_ );
}

//-----------------------------------------------------------------------------
int Viewer::get_num_rows_visible()
{
  return this->tile_layout_height_;
}

//-----------------------------------------------------------------------------
void Viewer::set_start_row( int row )
{
  this->start_row_ = row;
  this->display_meshes();
  this->render_window_->Render();
}

//-----------------------------------------------------------------------------
void Viewer::set_meshes( std::vector<QSharedPointer<Mesh> > meshes )
{
  this->meshes_ = meshes;
  this->display_meshes();
}
